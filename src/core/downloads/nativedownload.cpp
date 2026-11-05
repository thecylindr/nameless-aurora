#include "../auth/browserauthmanager.h"
#include "../auth/webloginmanager.h"
#include "../librarymodel.h"
#include "../settingsstore.h"
#include "../web/mediasniffer.h"
#include "dependencymanager.h"
#include "nativejob_p.h"
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QTimer>

// «Нативные» загрузки VK/Яндекс Музыки: скачивание аудио по прямой ссылке через QNetworkAccessManager,
// конвертация/проверка файла FFmpeg и запись в библиотеку.
void DownloadManager::enqueueNative(const QVariantList& tracks, const QString& quality)
{
    if (!m_library || !m_settings) {
        setLastError(tr("Библиотека недоступна."));
        return;
    }
    if (!QStringList{"original", "balanced", "compact"}.contains(quality)) {
        setLastError(tr("Выбери качество загрузки."));
        return;
    }
    QList<int> planned;
    for (const auto& entry : tracks)
        planned.append(entry.toMap().value("trackId").toInt());
    registerTracks(planned);
    setLastError({});
    for (const auto& entry : tracks) {
        const auto track = entry.toMap();
        const QString url = track.value("url").toString();
        const QString service = serviceForUrl(url);
        if (service != "VK" && service != "Яндекс Музыка")
            continue;
        bool duplicate = false;
        for (const auto* job : std::as_const(m_jobs))
            if (job->url == url && isActive(job)) {
                duplicate = true;
                break;
            }
        if (duplicate)
            continue;
        const int trackId = track.value("trackId").toInt();
        if (m_library && trackId > 0 && QFileInfo::exists(m_library->repository().track(trackId).filePath))
            continue;
        auto* job = new Job;
        job->id = m_nextJobId++;
        job->native = true;
        job->webTrack = track;
        job->url = url;
        job->service = service;
        job->quality = quality;
        job->title = track.value("artist").toString() + " — " + track.value("title").toString();
        const int row = m_jobs.size();
        beginInsertRows({}, row, row);
        m_jobs.append(job);
        endInsertRows();
    }
    updateAggregate();
    startQueued();
}
void DownloadManager::startNative(Job* job)
{
    const QUrl direct(job->webTrack.value("mediaUrl").toString());
    if (MediaSniffer::isAudioUrl(direct) ||
        (job->webTrack.value("mediaVerified").toBool() && direct.scheme() == "https" && direct.userInfo().isEmpty())) {
        job->nativeFailure.clear();
        job->cancelRequested = false;
        downloadNative(job, direct);
        return;
    }
    if (!m_auth || !m_auth->webLogin()) {
        finishFailed(job, tr("Модуль входа недоступен."));
        return;
    }
    job->nativeFailure.clear();
    job->cancelRequested = false;
    setJobStatus(job, "resolving");
    setStatusText(tr("Получаем аудио из %1…").arg(job->service));
    m_auth->webLogin()->resolveMedia(job->id, job->webTrack);
}
void DownloadManager::cleanupNative(Job* job)
{
    if (job->native && job->process && job->process->state() != QProcess::NotRunning) {
        disconnect(job->process, nullptr, this, nullptr);
        job->process->kill();
        job->process->waitForFinished(1000);
    }
    if (job->reply) {
        auto* reply = job->reply;
        job->reply = nullptr;
        disconnect(reply, nullptr, this, nullptr);
        reply->abort();
        reply->deleteLater();
    }
    if (job->output) {
        job->output->close();
        delete job->output;
        job->output = nullptr;
    }
}
void DownloadManager::downloadNative(Job* job, const QUrl& url)
{
    if (!MediaSniffer::isAudioUrl(url) &&
        !(job->webTrack.value("mediaVerified").toBool() && url.scheme() == "https" && url.userInfo().isEmpty())) {
        finishFailed(job, tr("Сервис не предоставил доступный аудиофайл."));
        return;
    }
    // HLS-поток (.m3u8) отдаём сразу в FFmpeg для склейки и конвертации.
    if (url.path().endsWith(".m3u8", Qt::CaseInsensitive)) {
        convertNative(job, url.toString(QUrl::FullyEncoded), true);
        return;
    }
    // mp4 от музыкальных сервисов — это, как правило, аудио в контейнере m4a.
    QString extension = QFileInfo(url.path()).suffix().toLower();
    if (extension == "mp4" || url.path().contains("/get-mp4/"))
        extension = "m4a";
    // Неизвестное расширение считаем mp3 — реальный формат уточним по MIME после скачивания.
    if (!QStringList{"mp3", "m4a", "webm", "aac", "ogg", "opus", "flac", "wav"}.contains(extension))
        extension = "mp3";
    job->nativePath = QDir(job->tempDir).filePath("source." + extension);
    job->output = new QFile(job->nativePath);
    if (!job->output->open(QIODevice::WriteOnly)) {
        finishFailed(job, tr("Не удалось создать аудиофайл. Проверь свободное место."));
        return;
    }
    QNetworkRequest request(url);
    request.setTransferTimeout(30000);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setMaximumRedirectsAllowed(5);
    QString agent = job->webTrack.value("userAgent").toString();
    if (agent.isEmpty() && m_auth && m_auth->webLogin())
        agent = m_auth->webLogin()->serviceUserAgent();
    if (agent.isEmpty())
        agent = "Mozilla/5.0 Aurora/4.0";
    request.setRawHeader("User-Agent", agent.toUtf8());
    request.setRawHeader(
        "Origin", QUrl(job->url)
                      .adjusted(QUrl::RemovePath | QUrl::RemoveQuery | QUrl::RemoveFragment | QUrl::RemoveUserInfo)
                      .toEncoded());
    request.setRawHeader("Referer", QUrl(job->url).adjusted(QUrl::RemoveQuery | QUrl::RemoveFragment).toEncoded());
    // Подписанные URL CDN не требуют cookies аккаунта — никогда не передаём cookies сервиса в CDN.
    job->reply = m_network->get(request);
    auto* reply = job->reply;
    const int id = job->id;
    setJobStatus(job, "downloading");
    connect(reply, &QNetworkReply::readyRead, this, [this, id, reply] {
        const int row = indexOfJob(id);
        if (row < 0)
            return;
        auto* current = m_jobs[row];
        if (current->reply != reply || !current->output)
            return;
        const auto bytes = reply->readAll();
        if (current->output->write(bytes) != bytes.size()) {
            current->nativeFailure = tr("Не удалось записать файл. Проверь свободное место.");
            reply->abort();
        }
    });
    connect(reply, &QNetworkReply::downloadProgress, this, [this, id, reply](qint64 received, qint64 total) {
        const int row = indexOfJob(id);
        if (row < 0)
            return;
        auto* current = m_jobs[row];
        if (current->reply != reply)
            return;
        // Защита от неадекватно больших файлов и от нехватки места на диске.
        if (received > 2LL * 1024 * 1024 * 1024) {
            current->nativeFailure = tr("Аудиофайл превышает 2 ГБ.");
            reply->abort();
            return;
        }
        const QStorageInfo storage(current->tempDir);
        if (storage.isValid() && storage.bytesAvailable() < 16 * 1024 * 1024) {
            current->nativeFailure = tr("На диске закончилось место.");
            reply->abort();
            return;
        }
        // Скачивание показываем до 92% — оставшееся отводим на проверку и импорт.
        if (total > 0)
            setJobProgress(current, qMin(0.92, double(received) / double(total) * 0.92));
    });
    connect(reply, &QNetworkReply::finished, this, [this, id, reply] {
        const int row = indexOfJob(id);
        if (row < 0)
            return;
        auto* current = m_jobs[row];
        if (current->reply != reply)
            return;
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const bool ok = reply->error() == QNetworkReply::NoError && status >= 200 && status < 300;
        const auto tail = reply->readAll();
        if (current->output && current->output->write(tail) != tail.size())
            current->nativeFailure = tr("Не удалось записать файл.");
        current->reply = nullptr;
        reply->deleteLater();
        if (current->output) {
            current->output->close();
            delete current->output;
            current->output = nullptr;
        }
        if (!ok || !current->nativeFailure.isEmpty()) {
            auto* web = m_auth ? m_auth->webLogin() : nullptr;
            if ((status == 401 || status == 403) && !current->nativeUrlRetried && web &&
                ((current->service == "VK" && web->vkLoggedIn()) ||
                 (current->service == "Яндекс Музыка" && web->yandexLoggedIn()))) {
                current->nativeUrlRetried = true;
                current->webTrack.remove("mediaUrl");
                current->webTrack.remove("mediaVerified");
                setStatusText("Ссылка истекла. Запрашиваем новую у сервиса…");
                startNative(current);
                return;
            }
            QString error = current->nativeFailure;
            if (error.isEmpty())
                error = status == 401 || status == 403
                            ? tr("Ссылка на аудио истекла или доступ ограничен. Нажми повтор: Aurora "
                                 "запросит новую ссылку.")
                            : tr("Аудио не загрузилось. Проверь соединение и повтори попытку.");
            finishFailed(current, error);
            return;
        }
        const auto mime =
            reply->header(QNetworkRequest::ContentTypeHeader).toString().section(';', 0, 0).trimmed().toLower();
        QFile sniff(current->nativePath);
        QByteArray start;
        if (sniff.open(QIODevice::ReadOnly))
            start = sniff.read(128);
        sniff.close();
        if (mime.contains("mpegurl") || start.trimmed().startsWith("#EXTM3U")) {
            QFile::remove(current->nativePath);
            convertNative(current, reply->url().toString(QUrl::FullyEncoded), true);
            return;
        }
        if (mime == "text/html" || mime == "application/json") {
            finishFailed(current,
                         "Вместо аудио источник вернул страницу или сообщение об ошибке. Пробуем другой вариант.");
            return;
        }
        const QHash<QString, QString> suffixes{{"audio/mpeg", "mp3"},  {"audio/mp4", "m4a"}, {"audio/ogg", "ogg"},
                                               {"audio/flac", "flac"}, {"audio/wav", "wav"}, {"audio/x-wav", "wav"},
                                               {"audio/webm", "webm"}};
        const auto suffix = suffixes.value(mime);
        if (!suffix.isEmpty() && QFileInfo(current->nativePath).suffix() != suffix) {
            const auto renamed = QDir(current->tempDir).filePath("source." + suffix);
            if (!QFile::rename(current->nativePath, renamed)) {
                finishFailed(current, "Не удалось подготовить загруженный файл.");
                return;
            }
            current->nativePath = renamed;
        }
        if (current->quality != "original")
            convertNative(current, current->nativePath, false);
        else
            finishSucceeded(current);
    });
    QTimer::singleShot(15 * 60 * 1000, reply, [reply] {
        if (!reply->isFinished())
            reply->abort();
    });
}
void DownloadManager::convertNative(Job* job, const QString& input, bool remote)
{
    const QString ffmpeg =
        m_dependencies ? m_dependencies->ffmpegExecutable() : QStandardPaths::findExecutable("ffmpeg");
    if (ffmpeg.isEmpty()) {
        finishFailed(job,
                     tr("Для потокового аудио и смены качества нужен FFmpeg. Установи его в разделе загрузчиков."));
        return;
    }
    const QString extension = job->quality == "original" ? "mka" : "mp3";
    const QString path = QDir(job->tempDir).filePath("audio." + extension);
    QStringList args{"-hide_banner", "-nostdin", "-loglevel", "error", "-y"};
    if (remote) {
        QString agent = job->webTrack.value("userAgent").toString();
        if (agent.isEmpty())
            agent = m_auth && m_auth->webLogin() ? m_auth->webLogin()->serviceUserAgent()
                                                 : QStringLiteral("AuroraPlayer/4.0");
        args << "-user_agent" << agent;
        args << "-headers"
             << ("Origin: " +
                 QUrl(job->url)
                     .adjusted(QUrl::RemovePath | QUrl::RemoveQuery | QUrl::RemoveFragment | QUrl::RemoveUserInfo)
                     .toString() +
                 "\r\n");
        args << "-protocol_whitelist" << "https,tls,tcp,crypto" << "-rw_timeout" << "30000000" << "-referer"
             << job->url;
    }
    args << "-i" << input << "-map" << "0:a:0" << "-vn";
    if (job->quality == "original")
        args << "-c:a" << "copy";
    else
        args << "-c:a" << "libmp3lame" << "-b:a" << (job->quality == "balanced" ? "192k" : "128k");
    args << "-metadata" << ("title=" + job->webTrack.value("title").toString()) << "-metadata"
         << ("artist=" + job->webTrack.value("artist").toString()) << path;
    auto* process = new QProcess(this);
    job->process = process;
    const int id = job->id;
    setJobStatus(job, "downloading");
    connect(process, &QProcess::readyReadStandardError, process, [process] {
        process->readAllStandardError();
    });
    connect(process, &QProcess::errorOccurred, this, [this, id](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart)
            return;
        const int row = indexOfJob(id);
        if (row >= 0)
            finishFailed(m_jobs[row], tr("Не удалось запустить обработку аудио."));
    });
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, id, input, remote](int code, QProcess::ExitStatus exit) {
                const int row = indexOfJob(id);
                if (row < 0)
                    return;
                auto* current = m_jobs[row];
                if (!current->process)
                    return;
                current->process->deleteLater();
                current->process = nullptr;
                if (current->cancelRequested) {
                    finishCancelled(current);
                    return;
                }
                if (code || exit != QProcess::NormalExit) {
                    finishFailed(
                        current,
                        tr("Поток недоступен или не поддерживается. Открой трек в сервисе и повтори загрузку."));
                    return;
                }
                if (!remote)
                    QFile::remove(input);
                finishSucceeded(current);
            });
    QTimer::singleShot(15 * 60 * 1000, process, [process] {
        if (process->state() != QProcess::NotRunning)
            process->kill();
    });
    process->start(ffmpeg, args);
}
bool DownloadManager::validateNativeFile(Job* job, const QString& path, QString* error)
{
    const QString ffprobe =
        m_dependencies ? m_dependencies->ffprobeExecutable() : QStandardPaths::findExecutable("ffprobe");
    const QString ffmpeg =
        m_dependencies ? m_dependencies->ffmpegExecutable() : QStandardPaths::findExecutable("ffmpeg");
    return validateAudioFile(path, job->webTrack.value("durationMs").toDouble(), ffprobe, ffmpeg, error);
}
bool DownloadManager::validateAudioFile(const QString& path, double expected, const QString& ffprobe,
                                        const QString& ffmpeg, QString* error)
{
    if (ffprobe.isEmpty()) {
        *error = tr("Для проверки аудио нужен ffprobe из пакета FFmpeg.");
        return false;
    }
    QProcess probe;
    probe.start(ffprobe, {"-v", "error", "-select_streams", "a:0", "-show_entries", "stream=codec_type:format=duration",
                          "-of", "json", path});
    if (!probe.waitForFinished(5000)) {
        probe.kill();
        probe.waitForFinished(1000);
        *error = tr("Не удалось проверить аудиофайл.");
        return false;
    }
    const auto document = QJsonDocument::fromJson(probe.readAllStandardOutput()).object();
    const double duration = document.value("format").toObject().value("duration").toString().toDouble() * 1000;
    if (probe.exitCode() != 0 || document.value("streams").toArray().isEmpty() || duration <= 0) {
        *error = tr("Вместо аудио сервис вернул пустой или неподдерживаемый файл.");
        return false;
    }
    if (expected > 15000 && duration + 5000 < expected * 0.85) {
        *error = tr("Сервис отдал только фрагмент трека. Полный трек не сохранён; проверь доступ в аккаунте.");
        return false;
    }
    // Метаданные контейнера могут выглядеть корректно даже при зашифрованных аудиопакетах.
    // Поэтому пробуем декодировать короткий фрагмент, прежде чем добавлять файл в библиотеку.
    if (ffmpeg.isEmpty()) {
        *error = tr("Для проверки воспроизводимости аудио нужен FFmpeg.");
        return false;
    }
    QProcess decoder;
    decoder.start(ffmpeg, {"-hide_banner", "-nostdin", "-v", "error", "-xerror", "-i", path, "-map", "0:a:0", "-t", "1",
                           "-f", "null", "-"});
    if (!decoder.waitForFinished(5000)) {
        decoder.kill();
        decoder.waitForFinished(1000);
        *error = tr("Не удалось проверить воспроизводимость аудиофайла.");
        return false;
    }
    if (decoder.exitStatus() != QProcess::NormalExit || decoder.exitCode() != 0) {
        *error = tr("Файл не декодируется: сервис мог вернуть защищённый поток вместо обычного аудио. Файл не добавлен "
                    "в медиатеку.");
        return false;
    }
    return true;
}
