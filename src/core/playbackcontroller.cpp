#include "playbackcontroller.h"
#include "librarymodel.h"
#include "settingsstore.h"
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <array>
#include <QRandomGenerator>
#include <QSettings>
#include <QAudioBuffer>
#include <QMediaDevices>
#include <QNetworkReply>
#include <QAudioDevice>
#include <cmath>
#include <algorithm>

// Реализация контроллера воспроизведения: связь плеера с эквалайзером,
// аудиореактивностью, очередью и учётом прослушиваний.
namespace
{
// Формат для анализа громкости; в headless-режиме (CI) используем запасной.
QAudioFormat meterFormat()
{
    if (!qEnvironmentVariableIsSet("AURORA_HEADLESS")) {
        const auto format = QMediaDevices::defaultAudioOutput().preferredFormat();
        if (format.isValid())
            return format;
    }
    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Float);
    return format;
}
} // namespace

PlaybackController::PlaybackController(LibraryModel* library, SettingsStore* settings, QObject* parent)
    : QObject(parent), m_library(library), m_settings(settings), m_bufferOutput(meterFormat())
{
    m_shuffle = QSettings().value("playback/shuffle", false).toBool();
    m_repeat = qBound(0, QSettings().value("playback/repeat", 0).toInt(), 2);
    m_volume = settings->volume();
    m_meterClock.start();
    m_bufferClock.start();
    m_player.setAudioBufferOutput(&m_bufferOutput);
    m_silenceTimer.setInterval(100);
    connect(&m_silenceTimer, &QTimer::timeout, this, [this] {
        if (m_bufferClock.elapsed() > 220 && (audioEnergy() > 0 || bassEnergy() > 0 || beatPulse() > 0))
            resetReactivity();
    });
    m_sinkTimer.setInterval(10);
    connect(&m_sinkTimer, &QTimer::timeout, this, &PlaybackController::pumpEqualizedAudio);
    connect(&m_equalizer, &AudioEqualizer::enabledChanged, this, [this] {
        const auto position = m_player.position();
        flushEqualizedAudio();
        // Оставляем выход Qt подключённым как «часы» воспроизведения. Отключение
        // позволяет FFmpeg-бэкенду декодировать вперёд отдельно обработанного выхода.
        if (m_audioOutput)
            m_audioOutput->setMuted(m_equalizer.enabled());
        if (hasTrack() && seekable())
            m_player.setPosition(position);
    });
    connect(&m_bufferOutput, &QAudioBufferOutput::audioBufferReceived, this, [this](const QAudioBuffer& input) {
        if (!playing())
            return;
        if (!input.isValid()) {
            resetReactivity();
            return;
        }
        const auto buffer = m_equalizer.process(input);
        if (m_equalizer.enabled())
            outputEqualizedAudio(buffer);
        if (!m_settings->motionEnabled())
            return;
        m_bufferClock.restart();
        m_reactivity.process(buffer);
        // Обновляем показатели не чаще ~30 раз в секунду.
        if (m_meterClock.elapsed() < 33)
            return;
        emit audioReactivityChanged();
        m_meterClock.restart();
        const auto format = buffer.format();
        const auto* data = buffer.constData<char>();
        const int count = buffer.sampleCount(), bytes = format.bytesPerSample();
        if (count < 1 || bytes < 1)
            return;
        QVariantList levels;
        // Разбиваем буфер на 24 «столбика» и считаем среднеквадратичную мощность.
        constexpr int bars = 24;
        for (int bar = 0; bar < bars; ++bar) {
            double power = 0;
            int n = 0;
            const int begin = bar * count / bars, end = (bar + 1) * count / bars;
            for (int i = begin; i < end; i += qMax(1, (end - begin) / 20)) {
                const double sample = format.normalizedSampleValue(data + i * bytes);
                power += sample * sample;
                ++n;
            }
            levels.append(n ? qBound(0.0, std::sqrt(power / n) * 3, 1.0) : 0.0);
        }
        m_audioLevels = levels;
        emit audioLevelsChanged();
    });
    // В headless-окружении (CI) нет аудиосервера; декодирование и «часы»
    // воспроизведения при этом продолжают работать.
    if (!qEnvironmentVariableIsSet("AURORA_HEADLESS")) {
        m_audioOutput = std::make_unique<QAudioOutput>();
        m_player.setAudioOutput(m_audioOutput.get());
        m_audioOutput->setMuted(m_equalizer.enabled());
        m_audioOutput->setVolume(float(m_volume));
    }
    connect(&m_player, &QMediaPlayer::positionChanged, this, &PlaybackController::positionChanged);
    connect(&m_player, &QMediaPlayer::durationChanged, this, &PlaybackController::durationChanged);
    connect(&m_player, &QMediaPlayer::seekableChanged, this, &PlaybackController::seekableChanged);
    connect(&m_player, &QMediaPlayer::playbackStateChanged, this, [this] {
        if (m_equalizedSink) {
            if (playing()) {
                m_equalizedSink->resume();
                m_sinkTimer.start();
            } else if (m_player.playbackState() == QMediaPlayer::PausedState) {
                m_equalizedSink->suspend();
                m_sinkTimer.stop();
            } else if (m_player.mediaStatus() != QMediaPlayer::EndOfMedia &&
                       !(duration() > 0 && position() >= duration() - 200))
                flushEqualizedAudio();
        }
        accountListen();
        m_counting = playing();
        m_elapsed.restart();
        if (playing() && m_settings->motionEnabled()) {
            m_bufferClock.restart();
            m_silenceTimer.start();
        } else {
            m_silenceTimer.stop();
            resetReactivity();
        }
        emit playingChanged();
    });
    connect(&m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus s) {
        emit streamLoadingChanged();
        if (s == QMediaPlayer::EndOfMedia) {
            const int generation = m_audioGeneration;
            int delay = 0;
            if (m_equalizedSink) {
                // Ждём, пока обработанный звук доиграет до конца, перед переходом.
                const auto bytes = m_pendingAudio.size() + m_equalizedSink->bufferSize() - m_equalizedSink->bytesFree();
                delay = int(std::clamp<qint64>(m_equalizedSink->format().durationForBytes(bytes) / 1000, 0, 500));
            }
            QTimer::singleShot(delay, this, [this, generation] {
                // Если за время задержки сменился трек — ничего не делаем.
                if (generation != m_audioGeneration)
                    return;
                commitListen(true);
                if (m_streaming) {
                    if (m_repeat == 2 && seekable()) {
                        seek(0);
                        play();
                    } else
                        stop();
                    return;
                }
                if (m_repeat == 2)
                    startTrack(m_currentTrackId);
                else
                    advance(true);
            });
        }
    });
    connect(&m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error, const QString& e) {
        setError(e);
        emit streamLoadingChanged();
    });
    connect(settings, &SettingsStore::volumeChanged, this, [this] {
        m_volume = m_settings->volume();
        if (m_audioOutput)
            m_audioOutput->setVolume(float(m_volume));
        emit volumeChanged();
        if (m_equalizedSink)
            m_equalizedSink->setVolume(m_volume);
    });
    connect(settings, &SettingsStore::appearanceChanged, this, [this] {
        if (!m_settings->motionEnabled()) {
            m_silenceTimer.stop();
            resetReactivity();
        } else if (playing())
            m_silenceTimer.start();
    });
    connect(library, &LibraryModel::libraryChanged, this, [this] {
        // Фильтр или сортировка не меняют состав и порядок играющей очереди.
        if (m_currentTrackId > 0 && !m_library->trackById(m_currentTrackId).id) {
            m_player.stop();
            m_currentTrackId = -1;
            m_listened = 0;
        }
        if (m_currentTrackId > 0) {
            const auto t = m_library->trackById(m_currentTrackId);
            const bool paletteChanged =
                m_currentTitle != t.title || m_currentArtist != t.artist || m_currentCoverPath != t.coverPath;
            m_currentTitle = t.title;
            m_currentArtist = t.artist;
            m_currentCoverPath = t.coverPath;
            m_currentFormat = t.format;
            if (paletteChanged)
                updateAtmosphere();
        }
        emit currentTrackChanged();
        emit streamLoadingChanged();
        emit queueChanged();
    });
}
PlaybackController::~PlaybackController()
{
    // Прерываем незавершённые сетевые запросы обложек и корректно закрываем плеер.
    for (auto* reply : m_artNetwork.findChildren<QNetworkReply*>()) {
        disconnect(reply, nullptr, this, nullptr);
        reply->abort();
    }
    flushEqualizedAudio();
    m_silenceTimer.stop();
    disconnect(&m_bufferOutput, nullptr, this, nullptr);
    disconnect(&m_player, nullptr, this, nullptr);
    commitListen(false);
    m_player.stop();
    m_player.setSource({});
    m_streamProxy.reset();
    m_player.setAudioBufferOutput(nullptr);
    m_player.setAudioOutput(nullptr);
}
// Останавливает отдельный выход эквалайзера и сбрасывает накопленный звук.
void PlaybackController::flushEqualizedAudio()
{
    ++m_audioGeneration;
    m_sinkTimer.stop();
    m_pendingAudio.clear();
    m_sinkWriter = nullptr;
    if (m_equalizedSink) {
        m_equalizedSink->reset();
        m_equalizedSink.reset();
    }
    m_equalizer.clearHistory();
}
// Создаёт отдельный аудиовыход для обработанного звука и копит данные в буфер.
void PlaybackController::outputEqualizedAudio(const QAudioBuffer& buffer)
{
    if (qEnvironmentVariableIsSet("AURORA_HEADLESS"))
        return;
    if (!m_equalizedSink) {
        m_equalizedSink = std::make_unique<QAudioSink>(QMediaDevices::defaultAudioOutput(), buffer.format());
        m_equalizedSink->setBufferSize(buffer.format().bytesForDuration(160000));
        m_equalizedSink->setVolume(m_volume);
        m_sinkWriter = m_equalizedSink->start();
        if (!m_sinkWriter) {
            m_equalizer.setEnabled(false);
            setError("Аудиоустройство не приняло обработанный звук. Эквалайзер отключён.");
            return;
        }
        m_sinkTimer.start();
    }
    m_pendingAudio.append(buffer.constData<char>(), buffer.byteCount());
    if (m_pendingAudio.size() > buffer.format().bytesForDuration(2000000)) {
        m_equalizer.setEnabled(false);
        setError("Аудиоустройство не успевает воспроизводить звук. Эквалайзер отключён.");
        return;
    }
    pumpEqualizedAudio();
}
// Записывает в аудиовыход ровно столько, сколько он готов принять.
void PlaybackController::pumpEqualizedAudio()
{
    if (!m_equalizedSink || !m_sinkWriter || m_pendingAudio.isEmpty())
        return;
    const auto size = qMin<qsizetype>(m_pendingAudio.size(), m_equalizedSink->bytesFree());
    if (size > 0) {
        const auto sent = m_sinkWriter->write(m_pendingAudio.constData(), size);
        if (sent > 0)
            m_pendingAudio.remove(0, sent);
        else if (sent < 0) {
            m_equalizer.setEnabled(false);
            setError("Аудиовыход закрыл поток обработки. Эквалайзер отключён; обычное воспроизведение доступно.");
        }
    }
}
void PlaybackController::resetReactivity()
{
    m_reactivity.reset();
    m_audioLevels.clear();
    emit audioReactivityChanged();
    emit audioLevelsChanged();
}
int PlaybackController::currentIndex() const
{
    return m_library->rowForTrackId(m_currentTrackId);
}
bool PlaybackController::canPlay() const
{
    return hasTrack() || !m_library->playableIds().isEmpty();
}
bool PlaybackController::favorite() const
{
    return m_library->trackById(m_currentTrackId).rating >= 4;
}
QVariantList PlaybackController::queue() const
{
    QVariantList ids;
    for (int id : m_queue)
        ids.append(id);
    return m_library->queueDetails(ids);
}
void PlaybackController::accountListen()
{
    if (m_counting && m_elapsed.isValid())
        m_listened += m_elapsed.elapsed();
    m_elapsed.restart();
}
void PlaybackController::commitListen(bool completed)
{
    accountListen();
    if (m_currentTrackId > 0 && m_listened > 0 && m_library->trackById(m_currentTrackId).id) {
        // Считаем «пропуском» прослушивание дольше 5 с, но короче 80 % трека.
        const bool skipped = !completed && m_listened > 5000 && duration() > 0 && m_listened < duration() * 0.8;
        m_library->recordListen(m_currentTrackId, m_listened, skipped);
    }
    m_listened = 0;
}
void PlaybackController::updateAtmosphere(const QImage& cover)
{
    // Детерминированные «семена» от заголовка и исполнителя (хэши FNV-1a),
    // чтобы цвет не прыгал при повторном открытии одного трека.
    quint32 hueSeed = 0, shapeSeed = 2166136261u;
    for (const auto ch : m_currentTitle)
        hueSeed = (hueSeed * 31 + ch.unicode()) & 0xffff;
    for (const auto ch : m_currentTitle + QChar(31) + m_currentArtist)
        shapeSeed = (shapeSeed ^ ch.unicode()) * 16777619u;
    m_atmosphereShape = double(shapeSeed % 997) / 997.;
    const double fallbackHue = double(hueSeed % 360) / 360.;
    m_atmosphereColor = QColor::fromHsvF(fallbackHue, .52, .78);
    m_atmosphereSecondary = QColor::fromHsvF(std::fmod(fallbackHue + .2, 1.), .42, .86);
    QImage image = cover;
    if (image.isNull()) {
        if (m_currentCoverPath.isEmpty() || m_currentCoverPath.startsWith("https://"))
            return;
        QImageReader reader(m_currentCoverPath);
        reader.setAutoTransform(true);
        reader.setScaledSize(QSize(48, 48));
        image = reader.read();
    } else
        image = image.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    if (image.isNull())
        return;
    // Раскладываем пиксели обложки по 24 корзинам оттенка (HSV) с весами.
    struct Bucket
    {
        double weight = 0, r = 0, g = 0, b = 0;
    };
    std::array<Bucket, 24> buckets{};
    for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x) {
            const auto c = image.pixelColor(x, y);
            // Пропускаем прозрачные, слишком блеклые или тёмные пиксели.
            if (c.alphaF() < .3 || c.hsvSaturationF() < .12 || c.valueF() < .08)
                continue;
            const int bin = std::clamp(int(c.hsvHueF() * 24), 0, 23);
            const double w = std::pow(c.hsvSaturationF(), 1.5) * c.valueF() * c.alphaF();
            auto& bucket = buckets[bin];
            bucket.weight += w;
            bucket.r += c.redF() * w;
            bucket.g += c.greenF() * w;
            bucket.b += c.blueF() * w;
        }
    int dominant = 0;
    for (int i = 1; i < 24; ++i)
        if (buckets[i].weight > buckets[dominant].weight)
            dominant = i;
    // Слишком мало «цветных» пикселей — используем спокойный сине-серый тон.
    if (buckets[dominant].weight < .05) {
        m_atmosphereColor = QColor::fromHsvF(.62, .16, .8);
        m_atmosphereSecondary = QColor::fromHsvF(.7, .12, .67);
        return;
    }
    const auto pigment = [&](int bin) {
        const auto& b = buckets[bin];
        const auto c = QColor::fromRgbF(b.r / b.weight, b.g / b.weight, b.b / b.weight);
        return QColor::fromHsvF(std::max(0., double(c.hsvHueF())), std::clamp(double(c.hsvSaturationF()), .22, .84),
                                std::clamp(double(c.valueF()), .68, .94));
    };
    m_atmosphereColor = pigment(dominant);
    // Вторичный цвет ищем в корзине, отстоящей минимум на 4 бина от доминирующей.
    int second = -1;
    for (int i = 0; i < 24; ++i) {
        const int distance = std::min(std::abs(i - dominant), 24 - std::abs(i - dominant));
        if (distance >= 4 && buckets[i].weight > buckets[dominant].weight * .12 &&
            (second < 0 || buckets[i].weight > buckets[second].weight))
            second = i;
    }
    m_atmosphereSecondary =
        second >= 0 ? pigment(second) : QColor::fromHsvF(std::fmod(m_atmosphereColor.hsvHueF() + .16, 1.), .46, .82);
}
// Начинает воспроизведение трека, предварительно проверив наличие аудиофайла.
bool PlaybackController::startTrack(int id)
{
    const Track t = m_library->trackById(id);
    if (t.id == 0 || t.filePath.isEmpty() || !QFileInfo::exists(t.filePath)) {
        setError("У этого трека нет аудиофайла. Привяжите файл или загрузите его по исходной ссылке.");
        return false;
    }
    commitListen(false);
    m_player.stop();
    flushEqualizedAudio();
    resetReactivity();
    m_listened = 0;
    m_streaming = false;
    m_streamCandidate.clear();
    m_player.setSource({});
    m_streamProxy.reset();
    m_currentTrackId = id;
    m_currentTitle = t.title;
    m_currentArtist = t.artist;
    m_currentFormat = t.format;
    m_currentCoverPath = t.coverPath;
    updateAtmosphere();
    setError({});
    m_player.setSource(QUrl::fromLocalFile(t.filePath));
    m_player.play();
    m_library->incrementPlayCount(id);
    emit currentTrackChanged();
    emit streamLoadingChanged();
    emit queueChanged();
    return true;
}
void PlaybackController::playIndex(int index)
{
    const auto i = m_library->index(index, 0);
    if (!i.isValid())
        return;
    playTrackId(m_library->data(i, LibraryModel::IdRole).toInt());
}
void PlaybackController::playStream(const QVariantMap& candidate)
{
    const QUrl remote(candidate.value("mediaUrl").toString());
    auto headers = candidate.value("streamHeaders").toMap();
    // Если поток не задал User-Agent, берём его из общих данных кандидата.
    if (!headers.contains("User-Agent") && candidate.contains("userAgent"))
        headers["User-Agent"] = candidate.value("userAgent");
    commitListen(false);
    m_player.stop();
    m_player.setSource({});
    flushEqualizedAudio();
    resetReactivity();
    const auto source = m_streamProxy.open(remote, headers, candidate.value("url").toString());
    if (source.isEmpty()) {
        m_streaming = false;
        m_streamCandidate.clear();
        m_currentTrackId = -1;
        setError("Не удалось открыть поток прослушивания. Повтори выбор записи.");
        emit currentTrackChanged();
        emit streamLoadingChanged();
        return;
    }
    m_streaming = true;
    m_streamCandidate = candidate;
    m_currentTrackId = -1;
    m_listened = 0;
    m_currentTitle = candidate.value("title").toString();
    m_currentArtist = candidate.value("artist").toString();
    m_currentCoverPath = candidate.value("coverUrl").toString();
    m_currentFormat = candidate.value("source").toString() + " · онлайн";
    updateAtmosphere();
    setError({});
    m_player.setSource(source);
    m_player.play();
    emit currentTrackChanged();
    emit streamLoadingChanged();
    emit durationChanged();
    emit queueChanged();
    const QUrl cover(m_currentCoverPath);
    if (cover.scheme() == "https") {
        // Подгружаем обложку потока отдельно, чтобы обновить атмосферу.
        QNetworkRequest request(cover);
        request.setTransferTimeout(7000);
        auto* reply = m_artNetwork.get(request);
        reply->setReadBufferSize(2 * 1024 * 1024 + 1);
        connect(reply, &QNetworkReply::readyRead, reply, [reply] {
            if (reply->bytesAvailable() > 2 * 1024 * 1024)
                reply->abort();
        });
        connect(reply, &QNetworkReply::finished, this, [this, reply, cover] {
            reply->deleteLater();
            if (!m_streaming || m_currentCoverPath != cover.toString() || reply->error() != QNetworkReply::NoError)
                return;
            const auto bytes = reply->readAll();
            if (bytes.size() > 2 * 1024 * 1024)
                return;
            QImage image;
            if (image.loadFromData(bytes)) {
                updateAtmosphere(image);
                emit currentTrackChanged();
                emit streamLoadingChanged();
            }
        });
    }
}
void PlaybackController::playTrackId(int id)
{
    if (id == m_currentTrackId) {
        toggle();
        return;
    }
    auto ids = m_library->playableIds();
    if (!ids.contains(id))
        ids.prepend(id);
    if (startTrack(id)) {
        m_queue = ids;
        m_history.clear();
        emit queueChanged();
    }
}
void PlaybackController::playQueueTrack(int id)
{
    if (id == m_currentTrackId) {
        toggle();
        return;
    }
    int previous = m_currentTrackId;
    if (startTrack(id) && previous > 0)
        m_history.append(previous);
}
void PlaybackController::playMix()
{
    QVector<int> ids;
    for (const QVariant& v : m_library->recommendations(50))
        ids.append(v.toMap().value("trackId").toInt());
    if (ids.isEmpty()) {
        setError("Добавьте аудиофайлы, чтобы запустить волну.");
        return;
    }
    if (startTrack(ids.first())) {
        m_queue = ids;
        m_history.clear();
        setShuffle(true);
        setRepeatMode(1);
        emit queueChanged();
    }
}
void PlaybackController::play()
{
    if (!hasTrack()) {
        auto ids = m_library->playableIds();
        if (!ids.isEmpty())
            playTrackId(ids.first());
    } else
        m_player.play();
}
void PlaybackController::pause()
{
    m_player.pause();
}
void PlaybackController::stop()
{
    commitListen(false);
    m_player.stop();
    flushEqualizedAudio();
}
void PlaybackController::toggle()
{
    if (playing())
        pause();
    else
        play();
}
// Переход к следующему треку с учётом перемешивания и режима повтора.
void PlaybackController::advance(bool natural)
{
    QVector<int> valid;
    // Отбрасываем из очереди треки без доступного аудиофайла.
    for (int id : m_queue) {
        const auto t = m_library->trackById(id);
        if (!t.filePath.isEmpty() && QFileInfo::exists(t.filePath))
            valid.append(id);
    }
    m_queue = valid;
    if (m_queue.isEmpty()) {
        stop();
        emit queueChanged();
        return;
    }
    int index = m_queue.indexOf(m_currentTrackId) + 1;
    if (m_shuffle && m_queue.size() > 1) {
        // Случайный трек, не повторяющий недавно прослушанные (кроме режима повтора).
        QVector<int> candidates;
        for (int id : m_queue)
            if (id != m_currentTrackId && !m_history.contains(id))
                candidates.append(id);
        if (candidates.isEmpty() && m_repeat != 0) {
            m_history.clear();
            for (int id : m_queue)
                if (id != m_currentTrackId)
                    candidates.append(id);
        }
        if (candidates.isEmpty()) {
            if (natural)
                stop();
            return;
        }
        index = m_queue.indexOf(candidates.at(QRandomGenerator::global()->bounded(candidates.size())));
    }
    if (index >= m_queue.size()) {
        if (m_repeat == 1)
            index = 0;
        else {
            if (natural)
                stop();
            return;
        }
    }
    int old = m_currentTrackId;
    if (startTrack(m_queue.at(index)) && old > 0)
        m_history.append(old);
}
void PlaybackController::next()
{
    advance(false);
}
void PlaybackController::previous()
{
    // Если трек играет дольше 3 секунд — просто перематываем к началу.
    if (position() > 3000) {
        seek(0);
        return;
    }
    if (!m_history.isEmpty()) {
        startTrack(m_history.takeLast());
        return;
    }
    int index = m_queue.indexOf(m_currentTrackId) - 1;
    if (index >= 0)
        startTrack(m_queue.at(index));
}
void PlaybackController::seek(qint64 value)
{
    // Перед перемоткой сбрасываем буфер эквалайзера, чтобы избежать «хвостов».
    flushEqualizedAudio();
    resetReactivity();
    m_player.setPosition(qBound<qint64>(0, value, qMax<qint64>(0, duration())));
    emit seeked(position());
}
void PlaybackController::toggleFavorite()
{
    if (m_currentTrackId > 0)
        m_library->setRating(m_currentTrackId, favorite() ? 0 : 5);
}
void PlaybackController::enqueue(int id)
{
    if (!m_queue.contains(id) && !m_library->trackById(id).filePath.isEmpty()) {
        m_queue.append(id);
        emit queueChanged();
    }
}
void PlaybackController::removeFromQueue(int id)
{
    if (id != m_currentTrackId) {
        m_queue.removeAll(id);
        emit queueChanged();
    }
}
void PlaybackController::setVolume(double value)
{
    m_settings->setVolume(value);
}
void PlaybackController::setShuffle(bool value)
{
    if (m_shuffle == value)
        return;
    m_shuffle = value;
    QSettings().setValue("playback/shuffle", value);
    emit modesChanged();
}
void PlaybackController::setRepeatMode(int value)
{
    value = qBound(0, value, 2);
    if (m_repeat == value)
        return;
    m_repeat = value;
    QSettings().setValue("playback/repeat", value);
    emit modesChanged();
}
void PlaybackController::setError(const QString& error)
{
    m_lastError = error;
    emit lastErrorChanged();
}
