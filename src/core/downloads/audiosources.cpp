#include "audiosources.h"
#include "opencatalog.h"
#include "zaycevcatalog.h"
#include <QDateTime>
#include "errormapper.h"
#include "dependencymanager.h"
#include "../auth/browsersession.h"
#include "../auth/webloginmanager.h"
#include "../downloadmanager.h"
#include "../librarymodel.h"
#include "../web/mediasniffer.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QSet>
#include <QUrlQuery>
#include <QSettings>
#include <algorithm>

// Реализация поиска музыки и автоподбора: каталоги Internet Archive/Zaycev.net,
// поиск через yt-dlp и браузерную сессию, а также выбор наилучшего кандидата.
namespace
{
// Определяет по тексту ошибки, что источник временно недоступен (403/429/таймаут/капча).
bool sourceOutage(const QString& message)
{
    const auto text = message.toLower();
    for (const auto& word : QStringList{"403", "429", "timeout", "не ответил", "не завершился", "соединени", "не робот",
                                        "cookies", "проверку доступа"})
        if (text.contains(word))
            return true;
    return false;
}
} // namespace

AudioSources::AudioSources(LibraryModel* library, DownloadManager* downloads, DependencyManager* tools, QObject* parent,
                           std::function<BrowserSession*(QObject*)> browserFactory, bool internalWorker)
    : QObject(parent), m_library(library), m_downloads(downloads), m_tools(tools),
      m_browserFactory(std::move(browserFactory))
{
    m_timer.setInterval(450);
    connect(&m_timer, &QTimer::timeout, this, &AudioSources::tick);
    m_internalWorker = internalWorker;
    m_automaticRecovery = !internalWorker && QSettings().value("downloads/automaticRecovery", true).toBool();
    m_recoverySources = QSettings()
                            .value("downloads/sourceOrder",
                                   QStringList{"YouTube", "SoundCloud", "Zaycev.net", "Hitmo", "Internet Archive"})
                            .toStringList();
    m_recoverySources.removeDuplicates();
    for (auto i = m_recoverySources.begin(); i != m_recoverySources.end();)
        if (!sources().contains(*i))
            i = m_recoverySources.erase(i);
        else
            ++i;
    m_extendedRecovery = m_recoverySources.contains("Hitmo") || m_recoverySources.contains("Zaycev.net");
    m_source = QSettings().value("downloads/preferredSearchSource", "YouTube").toString();
    if (!sources().contains(m_source))
        m_source = "YouTube";
    if (m_tools)
        connect(m_tools, &DependencyManager::preparationFinished, this, [this](bool success) {
            if (!m_waitingTools)
                return;
            m_waitingTools = false;
            if (success)
                search(m_source, m_query);
            else
                finish("Не удалось подготовить загрузку. " + m_tools->lastError());
        });
    if (!internalWorker && m_downloads)
        connect(m_downloads, &DownloadManager::trackTransferFinished, this,
                [this](int id, bool success, const QString& source, const QString& error) {
                    if (id == m_recoveryId && m_recoveryPhase == Downloading) {
                        if (success || error == "cancelled")
                            finishRecoveryTrack();
                        else {
                            if (sourceOutage(error)) {
                                if (++m_sourceFailures[source] >= 2)
                                    m_sourceCooldown[source] = QDateTime::currentMSecsSinceEpoch() + 60000;
                                m_recoverySourceAttempts[source] = 3;
                                m_recoveryPhase = Searching;
                                QTimer::singleShot(0, this, &AudioSources::recoveryStep);
                            } else if (m_recoveryAttempts < 15) {
                                m_recoveryPhase = Searching;
                                QTimer::singleShot(0, this, &AudioSources::recoveryStep);
                            } else
                                nextRecoverySource();
                        }
                    } else if (!success && m_automaticRecovery && !source.isEmpty() && error != "cancelled" &&
                               !error.contains("подготовить"))
                        recoverTrack(id);
                });
}
AudioSources::~AudioSources()
{
    stopWork();
}
void AudioSources::stopWork(bool keepBrowser)
{
    m_searchAll = false;
    m_searchQueue.clear();
    m_searchPending.clear();
    for (auto* worker : m_searchWorkers)
        worker->cancel();
    ++m_generation;
    ++m_scriptId;
    m_timer.stop();
    m_ready = false;
    m_pending = false;
    m_waitingTools = false;
    if (m_process) {
        disconnect(m_process, nullptr, this, nullptr);
        m_process->kill();
        m_process->waitForFinished(500);
        m_process->deleteLater();
        m_process = nullptr;
    }
    if (m_browser) {
        if (keepBrowser)
            m_browser->hide();
        else
            m_browser->stop();
    }
    if (m_catalog)
        m_catalog->cancel();
    if (m_zaycev)
        m_zaycev->cancel();
}
void AudioSources::cancel()
{
    m_preview = false;
    stopWork(true);
    m_busy = false;
    emit changed();
}
void AudioSources::finish(const QString& status)
{
    const bool preview = m_preview;
    m_preview = false;
    stopWork(true);
    m_status = status;
    m_busy = false;
    emit changed();
    emit operationFinished();
    if (preview)
        emit previewResolved({}, status);
}
void AudioSources::openTrack(int id)
{
    m_catalogMode = false;
    m_dialogOpen = true;
    if (m_worker && m_recoveryId > 0 && m_recoveryPhase != Downloading) {
        const int active = m_recoveryId;
        m_recoveryId = 0;
        m_recoveryPhase = Idle;
        m_worker->cancel();
        m_recoveryQueue.prepend(active);
    }
    cancel();
    m_target = m_library ? m_library->trackDetails(id) : QVariantMap{};
    m_results.clear();
    m_status = "Выбери источник и проверь найденную версию перед скачиванием.";
    emit targetChanged();
    emit resultsChanged();
    emit changed();
    if (!m_target.isEmpty())
        emit opened();
    const int generation = m_generation;
    if (!m_internalWorker)
        QTimer::singleShot(0, this, [this, generation, id] {
            if (m_dialogOpen && generation == m_generation && !m_busy && m_target.value("trackId").toInt() == id)
                searchAll();
        });
}
void AudioSources::searchCatalog(const QString& source, const QString& query)
{
    m_catalogMode = true;
    m_target = {{"title", query.trimmed()}};
    search(source, query);
}
AudioSources* AudioSources::createWorker(QObject* parent) const
{
    return new AudioSources(m_library, m_downloads, m_tools, parent, m_browserFactory, true);
}

void AudioSources::searchMatches(const QVariantMap& target)
{
    cancel();
    m_catalogMode = false;
    m_target = target;
    // Read current enabled sources for each online request, without changing preferences.
    const auto enabled = QSettings().value("downloads/sourceOrder", m_recoverySources).toStringList();
    m_recoverySources.clear();
    for (const auto& source : enabled)
        if (sources().contains(source) && !m_recoverySources.contains(source))
            m_recoverySources.append(source);
    searchAll();
}

bool AudioSources::automaticMatch(const QVariantMap& target, const QVariantMap& candidate)
{
    if (target.value("title").toString().trimmed().isEmpty())
        return false;
    const int score = matchScore(target, candidate);
    const auto expected = target.value("durationMs").toLongLong();
    const auto duration = candidate.value("durationMs").toLongLong();
    if (target.value("artist").toString().trimmed().isEmpty() && (expected <= 0 || duration <= 0))
        return false;
    if (expected > 0 && duration > 0)
        return score >= 90 && qAbs(expected - duration) <= qMax<qint64>(8000, expected / 15);
    return score == 100;
}

void AudioSources::downloadCandidate(const QVariantMap& candidate)
{
    if (m_busy)
        return;
    m_catalogMode = true;
    m_source = candidate.value("source").toString();
    m_results = {candidate};
    choose(0);
}
QString AudioSources::script(const QString& name) const
{
    QFile file(":/native/scripts/" + name);
    return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.readAll()) : QString();
}
int AudioSources::matchScore(const QVariantMap& target, const QVariantMap& candidate)
{
    // Нормализуем текст: убираем диакритику и приводим редкие буквы (ø→o, ł→l), затем разбиваем на слова.
    const auto words = [](QString value) {
        value = value.normalized(QString::NormalizationForm_KD).toCaseFolded();
        value.replace(QRegularExpression("\\p{M}"), "");
        value.replace(QChar(0x00f8), 'o');
        value.replace(QChar(0x0142), 'l');
        auto result = value.split(QRegularExpression("[^\\p{L}\\p{N}]+"), Qt::SkipEmptyParts);
        result.removeDuplicates();
        return result;
    };
    const auto wanted = words(target.value("artist").toString() + " " + target.value("title").toString());
    const auto actual = words(candidate.value("artist").toString() + " " + candidate.value("title").toString());
    int matched = 0;
    for (const auto& word : wanted)
        if (actual.contains(word))
            ++matched;
    int score = wanted.isEmpty() ? 0 : matched * 100 / wanted.size();
    const auto a = target.value("durationMs").toLongLong(), b = candidate.value("durationMs").toLongLong();
    if (a > 0 && b > 0 && qAbs(a - b) > qMax<qint64>(8000, a / 10))
        score = qMin(score, 55);
    const auto targetWords = words(target.value("title").toString()),
               candidateWords = words(candidate.value("title").toString());
    // Расхождение по «версии» (remix/cover/live и т. п.) сильно снижает доверие к совпадению.
    for (const auto& version : QStringList{"slowed", "sped", "remix", "cover", "live", "караоке", "remaster"})
        if (candidateWords.contains(version) != targetWords.contains(version))
            score = qMin(score, 50);
    return score;
}
void AudioSources::acceptResults(const QVariantList& candidates)
{
    if (!candidates.isEmpty()) {
        if (m_searchCache.size() >= 24)
            m_searchCache.erase(m_searchCache.begin());
        m_searchCache[m_source + '\n' + m_query.toCaseFolded()] = {QDateTime::currentMSecsSinceEpoch(), candidates};
    }
    m_results.clear();
    QSet<QString> seen;
    for (const auto& value : candidates) {
        auto c = value.toMap();
        const auto url = c.value("url").toString();
        if (url.isEmpty() || seen.contains(url) || m_downloads->serviceForUrl(url).isEmpty())
            continue;
        seen.insert(url);
        c["score"] = matchScore(m_target, c);
        c["source"] = m_source;
        m_results.append(c);
        // Ограничиваем выдачу: 36 вариантов для каталога, 12 для подбора трека.
        if (m_results.size() == (m_catalogMode ? 36 : 12))
            break;
    }
    if (!m_catalogMode)
        std::stable_sort(m_results.begin(), m_results.end(), [](const QVariant& a, const QVariant& b) {
            return a.toMap().value("score").toInt() > b.toMap().value("score").toInt();
        });
    emit resultsChanged();
    finish(m_results.isEmpty() ? "Совпадений не найдено. Попробуй другое название или источник."
                               : "Выбери подходящую запись. Название, версия и длительность могут отличаться.");
}
void AudioSources::search(const QString& source, const QString& query)
{
    if (source == "Все включённые") {
        searchAll(query);
        return;
    }
    if (m_target.isEmpty() || !sources().contains(source))
        return;
    const auto requested =
        query.trimmed().isEmpty()
            ? (m_target.value("artist").toString() + " " + m_target.value("title").toString()).left(250)
            : query.trimmed().left(250);
    if (m_busy && !m_resolving && source == m_source && requested == m_query)
        return;
    cancel();
    m_source = source;
    m_query = query.trimmed().isEmpty() ? m_target.value("artist").toString() + " " + m_target.value("title").toString()
                                        : query.trimmed();
    if (!m_internalWorker)
        QSettings().setValue("downloads/preferredSearchSource", source);
    m_query = m_query.left(250);
    m_results.clear();
    emit resultsChanged();
    m_resolving = false;
    m_submitted = false;
    m_busy = true;
    m_status = "Ищем в " + source + "…";
    m_elapsed.start();
    emit changed();
    m_searchError.clear();
    // Повторный запрос в течение 45 секунд берём из кэша, чтобы не дёргать источник.
    const auto cache = m_searchCache.value(m_source + '\n' + m_query.toCaseFolded());
    if (cache.first > 0 && QDateTime::currentMSecsSinceEpoch() - cache.first < 45000) {
        acceptResults(cache.second);
        return;
    }
    if (source == "Zaycev.net" && !m_browserFactory) {
        ensureZaycev();
        m_zaycev->search(m_query);
        return;
    }
    if (source == "Internet Archive") {
        if (!m_catalog) {
            m_catalog = new OpenCatalog(this);
            connect(m_catalog, &OpenCatalog::ready, this, [this](const QVariantList& tracks) {
                if (m_busy)
                    acceptResults(tracks);
            });
            connect(m_catalog, &OpenCatalog::failed, this, [this](const QString& error) {
                if (m_busy)
                    finish(error);
            });
        }
        m_catalog->search(query.trimmed().isEmpty() ? m_target.value("title").toString() : m_query,
                          m_target.value("artist").toString());
        return;
    }
    if (source == "YouTube" || source == "SoundCloud") {
        if (m_tools && !m_tools->runtimeReady()) {
            m_waitingTools = true;
            m_status = "Подготавливаем загрузку автоматически…";
            emit changed();
            m_tools->ensureReady();
            return;
        }
        const auto program = m_tools ? m_tools->ytDlpExecutable() : QString();
        if (program.isEmpty()) {
            finish("Для поиска установи загрузчик кнопкой ниже и повтори поиск.");
            return;
        }
        m_process = new QProcess(this);
        m_output.clear();
        const int generation = m_generation;
        connect(m_process, &QProcess::readyReadStandardOutput, this, [this] {
            m_output += m_process->readAllStandardOutput();
            if (m_output.size() > 4 * 1024 * 1024)
                finish("Источник вернул слишком большой список.");
        });
        connect(m_process, &QProcess::readyReadStandardError, this, [this] {
            if (m_process)
                m_searchError = (m_searchError + QString::fromLocal8Bit(m_process->readAllStandardError())).right(8192);
        });
        connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart)
                finish("Не удалось запустить поиск. Проверь загрузчик.");
        });
        connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
                [this, generation](int code, QProcess::ExitStatus state) {
                    if (generation != m_generation || !m_process)
                        return;
                    m_output += m_process->readAllStandardOutput();
                    m_searchError =
                        (m_searchError + QString::fromLocal8Bit(m_process->readAllStandardError())).right(8192);
                    if (code || state != QProcess::NormalExit) {
                        const auto reason = DownloaderErrors::humanizeDownloaderError(m_searchError);
                        finish(reason.isEmpty() ? "Источник не ответил. Попробуй другой или повтори позже." : reason);
                        return;
                    }
                    const auto root = QJsonDocument::fromJson(m_output).object();
                    QVariantList results;
                    for (const auto& entry : root.value("entries").toArray()) {
                        const auto e = entry.toObject();
                        QString url = e.value("webpage_url").toString(e.value("url").toString());
                        if (m_source == "YouTube" && !url.startsWith("http"))
                            url = "https://www.youtube.com/watch?v=" + e.value("id").toString();
                        results.append(QVariantMap{
                            {"title", e.value("title").toString()},
                            {"artist",
                             e.value("artist").toString(e.value("uploader").toString(e.value("channel").toString()))},
                            {"url", url},
                            {"coverUrl",
                             e.value("thumbnail")
                                 .toString(
                                     e.value("thumbnails").toArray().isEmpty()
                                         ? QString()
                                         : e.value("thumbnails").toArray().last().toObject().value("url").toString())},
                            {"durationMs", qRound64(e.value("duration").toDouble() * 1000)}});
                    }
                    acceptResults(results);
                });
        QStringList arguments{"--ignore-config",    "--flat-playlist",
                              "--dump-single-json", "--skip-download",
                              "--socket-timeout",   "10",
                              "--retries",          "1"};
        if (m_tools)
            arguments.append(m_tools->jsRuntimeArguments());
        arguments << "--"
                  << ((source == "YouTube" ? (m_catalogMode ? "ytsearch36:" : "ytsearch8:")
                                           : (m_catalogMode ? "scsearch36:" : "scsearch8:")) +
                      m_query);
        m_process->start(program, arguments);
        m_timer.start();
        return;
    }
    searchPublicBrowser();
}

void AudioSources::searchAll(const QString& query)
{
    if (m_target.isEmpty())
        return;
    cancel();
    m_query = query.trimmed().isEmpty()
                  ? (m_target.value("artist").toString() + " " + m_target.value("title").toString()).trimmed()
                  : query.trimmed().left(250);
    m_source = "Все включённые";
    m_results.clear();
    m_searchFailures.clear();
    m_searchAll = true;
    m_busy = true;
    for (const auto& source : m_recoverySources) {
        if (m_sourceCooldown.value(source) > QDateTime::currentMSecsSinceEpoch())
            continue;
        m_searchQueue.enqueue(source);
    }
    m_status = "Ищем во всех включённых источниках…";
    emit resultsChanged();
    emit changed();
    pumpSearches();
}

void AudioSources::pumpSearches()
{
    if (!m_searchAll)
        return;
    // Ограничиваем одновременные запросы к браузерам/процессам, даже если в очереди тысячи треков.
    while (m_searchPending.size() < 3 && !m_searchQueue.isEmpty()) {
        const auto source = m_searchQueue.dequeue();
        auto* worker = m_searchWorkers.value(source);
        if (!worker) {
            worker = new AudioSources(m_library, m_downloads, m_tools, this, m_browserFactory, true);
            m_searchWorkers.insert(source, worker);
            connect(worker, &AudioSources::operationFinished, this, [this, source] {
                receiveSearch(source);
            });
        }
        worker->m_target = m_target;
        worker->m_catalogMode = false;
        m_searchPending.insert(source);
        worker->search(source, m_query);
    }
    if (!m_searchPending.isEmpty() || !m_searchQueue.isEmpty())
        return;
    m_searchAll = false;
    m_busy = false;
    m_status = m_results.isEmpty()
                   ? (m_searchFailures.isEmpty() ? "Включи источники автоподбора в настройках."
                                                 : "Совпадений нет. " + m_searchFailures.join(" · "))
                   : QString("Найдено %1 вариантов из включённых источников. Проверь версию перед скачиванием.")
                         .arg(m_results.size());
    emit changed();
    emit operationFinished();
}

void AudioSources::receiveSearch(const QString& source)
{
    if (!m_searchAll || !m_searchPending.remove(source))
        return;
    auto* worker = m_searchWorkers.value(source);
    if (worker->results().isEmpty()) {
        m_searchFailures.append(source + ": " + worker->status());
        if (sourceOutage(worker->status()) && ++m_sourceFailures[source] >= 2)
            m_sourceCooldown[source] = QDateTime::currentMSecsSinceEpoch() + 60000;
    }
    QSet<QString> urls;
    for (const auto& entry : std::as_const(m_results))
        urls.insert(entry.toMap().value("url").toString());
    for (const auto& entry : worker->results()) {
        const auto url = entry.toMap().value("url").toString();
        if (!urls.contains(url)) {
            urls.insert(url);
            m_results.append(entry);
        }
    }
    std::stable_sort(m_results.begin(), m_results.end(), [](const QVariant& a, const QVariant& b) {
        return a.toMap().value("score").toInt() > b.toMap().value("score").toInt();
    });
    m_status = QString("Найдено %1 · продолжаем поиск в других источниках…").arg(m_results.size());
    emit resultsChanged();
    emit changed();
    QTimer::singleShot(0, this, &AudioSources::pumpSearches);
}

void AudioSources::ensureZaycev()
{
    if (!m_zaycev) {
        m_zaycev = new ZaycevCatalog(this);
        connect(m_zaycev, &ZaycevCatalog::ready, this, [this](const QVariantList& tracks) {
            if (m_busy && m_source == "Zaycev.net")
                acceptResults(tracks);
        });
        connect(m_zaycev, &ZaycevCatalog::resolved, this, [this](const QVariantMap& track) {
            if (m_busy && m_source == "Zaycev.net")
                queue(track);
        });
        connect(m_zaycev, &ZaycevCatalog::failed, this, [this](const QString& reason) {
            if (!m_busy || m_source != "Zaycev.net")
                return;
            if (reason.contains("ограничил")) {
                finish(reason);
                return;
            }
            if (m_resolving) {
                m_ready = false;
                m_pending = false;
                searchPublicBrowser();
            } else {
                m_status = "Читаем варианты со страницы Zaycev…";
                emit changed();
                searchPublicBrowser();
            }
        });
    }
}
void AudioSources::searchPublicBrowser()
{
    const auto browsers = WebLoginManager::detectBrowserExecutables();
    if (browsers.isEmpty()) {
        finish("Для этого источника нужен установленный Chromium-браузер.");
        return;
    }
    if (!m_browser) {
        m_browser = m_browserFactory ? m_browserFactory(this) : new BrowserSession(this);
        connect(m_browser, &BrowserSession::failed, this, [this](const QString& error) {
            if (m_busy && m_timer.isActive() && !m_process && (m_source == "Hitmo" || m_source == "Zaycev.net"))
                finish(error);
        });
        connect(m_browser, &BrowserSession::loaded, this, [this](const QUrl& url, bool ok, const QString& error) {
            if (!m_busy)
                return;
            // Навигация формы может уничтожить контекст выполнения до возврата результата.
            // Поэтому распознаём итоговый URL поиска по хосту.
            const auto host = url.host();
            const bool sameSource = m_source == "Hitmo" ? (host == "hitmos.fm" || host.endsWith(".hitmos.fm"))
                                                        : (host == "zaycev.net" || host.endsWith(".zaycev.net"));
            if (!sameSource)
                return;
            if (!ok) {
                finish(error);
                return;
            }
            m_ready = true;
            if (!m_resolving && sameSource) {
                if (url.path().contains("/search", Qt::CaseInsensitive))
                    m_submitted = true;
                for (const auto& item : QUrlQuery(url).queryItems(QUrl::FullyDecoded))
                    if (QString(item.second).replace('+', ' ').compare(m_query, Qt::CaseInsensitive) == 0)
                        m_submitted = true;
            }
        });
        connect(m_browser, &BrowserSession::audioRequested, this, [this](const QUrl& url) {
            if (m_busy && m_resolving) {
                auto candidate = m_candidate;
                candidate["mediaUrl"] = url.toString();
                candidate["mediaVerified"] = true;
                queue(candidate);
            }
        });
        connect(m_browser, &BrowserSession::evaluated, this, [this](int id, const QVariant& value) {
            if (!m_busy || id != m_scriptId)
                return;
            m_pending = false;
            if (!value.isValid())
                return;
            const auto root = QJsonDocument::fromJson(value.toString().toUtf8()).object();
            if (root.value("blocked").toBool()) {
                finish("Источник показал проверку доступа. Выбери другой источник или открой сайт самостоятельно.");
                return;
            }
            if (root.value("accessDenied").toBool()) {
                finish("Сервис ограничил загрузку этой записи. Выбери другой вариант или источник.");
                return;
            }
            if (m_resolving) {
                const QUrl media(root.value("mediaUrl").toString());
                if (MediaSniffer::isAudioUrl(media) ||
                    (root.value("mediaVerified").toBool() && media.scheme() == "https" && media.userInfo().isEmpty())) {
                    auto c = m_candidate;
                    c["mediaUrl"] = media.toString();
                    c["mediaVerified"] = root.value("mediaVerified").toBool();
                    queue(c);
                }
                return;
            }
            if (root.value("submitted").toBool())
                m_submitted = true;
            const auto results = root.value("tracks").toArray();
            if (m_submitted && !results.isEmpty())
                acceptResults(results.toVariantList());
            else if (root.value("apiFinished").toBool() && !root.value("loading").toBool() &&
                     m_elapsed.elapsed() > 12000)
                finish("Zaycev не вернул вариантов по этому запросу. Попробуй короче название или другой источник.");
        });
    }
    // Текущие публичные клиенты открывают сразу маршрут поиска; так мы не теряем
    // «синтетическую» отправку формы при инициализации страницы.
    QUrl page(m_source == "Hitmo" ? "https://rus.hitmos.fm/search" : "https://zaycev.net/search");
    QUrlQuery searchQuery;
    searchQuery.addQueryItem(m_source == "Hitmo" ? "q" : "query_search",
                             m_source == "Hitmo" ? m_query.toLower() : m_query);
    page.setQuery(searchQuery);
    if (m_resolving)
        page = QUrl(m_candidate.value("url").toString());
    m_timer.start();
    m_browser->open(browsers.first(), m_source == "Hitmo" ? "hitmo" : "zaycev", page, {}, true);
}
void AudioSources::tick()
{
    if (!m_busy)
        return;
    const int limit = m_resolving || m_source == "Hitmo" || m_source == "Zaycev.net" ? 25000 : 45000;
    if (m_elapsed.elapsed() > limit) {
        finish(m_resolving ? "Источник не отдал аудио за 25 секунд. Выбери другой вариант."
                           : QString("Поиск не завершился за %1 секунд. Выбери другой источник.").arg(limit / 1000));
        return;
    }
    if (m_process || !m_ready || m_pending || !m_browser)
        return;
    auto code = script("public-source.js");
    const auto args =
        QJsonObject{{"query", m_query},
                    {"submitted", m_submitted},
                    {"resolve", m_resolving},
                    {"source", m_source},
                    {"candidate", QJsonObject{{"downloadHash", m_candidate.value("downloadHash").toString()},
                                              {"streamingHash", m_candidate.value("streamingHash").toString()}}}};
    // Подставляем параметры запроса в JS-скрипт страницы и выполняем его в браузере.
    code.replace("__OPTIONS__", QString::fromUtf8(QJsonDocument(args).toJson(QJsonDocument::Compact)));
    m_pending = true;
    m_browser->evaluate(++m_scriptId, code, m_resolving);
}
void AudioSources::choose(int index)
{
    if (index < 0 || index >= m_results.size() || (m_busy && !m_searchAll))
        return;
    if (m_searchAll) {
        stopWork(true);
        m_busy = false;
    }
    m_candidate = m_results[index].toMap();
    m_source = m_candidate.value("source", m_source).toString();
    if (m_candidate.value("mediaVerified").toBool() &&
        QUrl(m_candidate.value("mediaUrl").toString()).scheme() == "https") {
        queue(m_candidate);
        return;
    }
    if (m_source == "YouTube" || m_source == "SoundCloud") {
        if (m_preview)
            resolveStream();
        else
            queue(m_candidate);
        return;
    }
    if (m_source == "Zaycev.net" && !m_browserFactory)
        ensureZaycev();
    if (m_source == "Zaycev.net" && m_zaycev &&
        (m_candidate.contains("downloadHash") || m_candidate.contains("streamingHash"))) {
        m_busy = true;
        m_resolving = true;
        m_elapsed.start();
        m_status = "Получаем файл из Zaycev…";
        emit changed();
        m_zaycev->resolve(m_candidate);
        return;
    }
    m_busy = true;
    m_resolving = true;
    m_ready = false;
    m_pending = false;
    m_status = "Получаем файл из " + m_source + "…";
    m_elapsed.start();
    emit changed();
    searchPublicBrowser();
}
void AudioSources::queue(const QVariantMap& candidate)
{
    auto entry = candidate;
    if (m_browser && entry.value("userAgent").toString().isEmpty())
        entry["userAgent"] = m_browser->userAgent();
    if (m_preview) {
        m_preview = false;
        finish("Аудио готово к прослушиванию.");
        emit previewResolved(entry, {});
        return;
    }
    int id = m_target.value("trackId").toInt();
    if (m_catalogMode) {
        Track track;
        track.title = candidate.value("title").toString();
        track.artist = candidate.value("artist").toString();
        track.album = candidate.value("album").toString();
        track.sourceUrl = candidate.value("url").toString();
        track.source = candidate.value("source", m_source).toString();
        track.duration = candidate.value("durationMs").toLongLong();
        id = m_library->repository().saveTrack(track);
        if (!id) {
            finish("Не удалось сохранить трек в библиотеку.");
            return;
        }
        m_library->refresh();
    }
    if (!m_downloads->enqueueAlternative(id, entry, {})) {
        finish("Трек уже скачан или находится в очереди. Проверь панель загрузок.");
        return;
    }
    m_library->cacheCover(id, QUrl(candidate.value("coverUrl").toString()));
    if (m_catalogMode)
        emit catalogQueued(id);
    finish("Трек добавлен в очередь загрузки.");
    emit queued();
}
void AudioSources::close()
{
    m_dialogOpen = false;
    cancel();
    QTimer::singleShot(0, this, &AudioSources::startRecovery);
}
void AudioSources::setAutomaticRecovery(bool value)
{
    if (value == m_automaticRecovery)
        return;
    m_automaticRecovery = value;
    QSettings().setValue("downloads/automaticRecovery", value);
    emit changed();
}
void AudioSources::setExtendedRecovery(bool value)
{
    auto order = m_recoverySources;
    for (const auto& source : QStringList{"Zaycev.net", "Hitmo"}) {
        if (value && !order.contains(source))
            order.append(source);
        else if (!value)
            order.removeAll(source);
    }
    setRecoverySources(order);
}
void AudioSources::setRecoverySources(const QStringList& sources)
{
    QStringList order;
    for (const auto& source : sources)
        if (this->sources().contains(source) && !order.contains(source))
            order.append(source);
    if (order == m_recoverySources)
        return;
    m_recoverySources = order;
    m_extendedRecovery = order.contains("Hitmo") || order.contains("Zaycev.net");
    QSettings().setValue("downloads/sourceOrder", order);
    emit changed();
}
void AudioSources::recoverMissing()
{
    if (!m_library)
        return;
    for (const auto& t : m_library->repository().tracks(m_library->searchText(), m_library->playlistFilter()))
        if (t.filePath.isEmpty() || !QFileInfo::exists(t.filePath))
            recoverTrack(t.id);
}
void AudioSources::recoverTrack(int id)
{
    if (m_internalWorker || !m_library || id <= 0 || m_recoveryIds.contains(id))
        return;
    const auto t = m_library->trackDetails(id);
    if (t.isEmpty() || t.value("downloaded").toBool())
        return;
    if (!recovering())
        m_sourceFailures.clear();
    m_downloads->registerTracks({id});
    m_recoveryIds.insert(id);
    m_recoveryQueue.enqueue(id);
    emit changed();
    QTimer::singleShot(0, this, &AudioSources::startRecovery);
}
void AudioSources::startRecovery()
{
    if (m_dialogOpen || m_recoveryId > 0 || m_recoveryQueue.isEmpty())
        return;
    if (!m_worker) {
        m_worker = new AudioSources(m_library, m_downloads, m_tools, this, m_browserFactory, true);
        connect(m_worker, &AudioSources::changed, this, [this] {
            QTimer::singleShot(0, this, &AudioSources::recoveryStep);
        });
        connect(m_worker, &AudioSources::queued, this, [this] {
            if (!m_recoveryId)
                return;
            m_recoveryPhase = Downloading;
            m_recoveryStatus =
                "Скачиваем «" + m_worker->target().value("title").toString() + "» · " + m_worker->source();
            emit changed();
            QTimer::singleShot(0, this, [this] {
                if (!m_recoveryId || m_recoveryPhase != Downloading)
                    return;
                const auto state = m_downloads->trackState(m_recoveryId).value("status").toString();
                if (state == "failed")
                    nextRecoverySource();
                else if (state == "cancelled")
                    finishRecoveryTrack();
            });
        });
    }
    m_recoveryId = m_recoveryQueue.dequeue();
    m_recoverySource = -1;
    m_worker->openTrack(m_recoveryId);
    nextRecoverySource();
}
void AudioSources::nextRecoverySource()
{
    if (!m_recoveryId)
        return;
    if (m_recoverySource >= 0) {
        finishRecoveryTrack();
        return;
    }
    m_recoverySource = 0;
    m_recoveryCandidate = -1;
    m_recoveryAttempts = 0;
    m_recoverySourceAttempts.clear();
    m_recoveryPhase = Searching;
    m_worker->m_recoverySources = m_recoverySources;
    m_worker->m_sourceCooldown = m_sourceCooldown;
    m_worker->searchAll();
    m_recoveryStatus = "Подбираем «" + m_worker->target().value("title").toString() + "» · все включённые источники";
    emit changed();
}
void AudioSources::recoveryStep()
{
    if (!m_recoveryId || !m_worker)
        return;
    if (m_library->trackDetails(m_recoveryId).value("downloaded").toBool()) {
        finishRecoveryTrack();
        return;
    }
    if (m_worker->busy())
        return;
    if (m_recoveryPhase == Searching) {
        const auto results = m_worker->results();
        int best = -1;
        for (int i = m_recoveryCandidate + 1; i < results.size(); ++i) {
            const auto c = results[i].toMap();
            if (m_recoverySourceAttempts.value(c.value("source").toString()) >= 3)
                continue;
            const auto expected = m_worker->target().value("durationMs").toLongLong(),
                       duration = c.value("durationMs").toLongLong();
            // Берём кандидата, если совпадение высокое (>=90) и длительность близка к ожидаемой.
            if ((duration > 0 && expected > 0 && c.value("score").toInt() >= 90 &&
                 qAbs(duration - expected) <= qMax<qint64>(8000, expected / 20)) ||
                ((duration <= 0 || expected <= 0) && c.value("score").toInt() == 100)) {
                best = i;
                break;
            }
        }
        if (best < 0) {
            if (results.isEmpty() && sourceOutage(m_worker->status()) && ++m_sourceFailures[m_worker->source()] >= 2)
                m_sourceCooldown[m_worker->source()] = QDateTime::currentMSecsSinceEpoch() + 60000;
            nextRecoverySource();
            return;
        }
        m_recoveryCandidate = best;
        ++m_recoverySourceAttempts[results[best].toMap().value("source").toString()];
        ++m_recoveryAttempts;
        m_recoveryPhase = Choosing;
        m_worker->choose(best);
    } else if (m_recoveryPhase == Choosing) {
        if (m_recoveryAttempts < 15) {
            m_recoveryPhase = Searching;
            QTimer::singleShot(0, this, &AudioSources::recoveryStep);
        } else
            nextRecoverySource();
    }
}
void AudioSources::finishRecoveryTrack()
{
    m_recoveryIds.remove(m_recoveryId);
    m_recoveryId = 0;
    m_recoveryPhase = Idle;
    if (m_worker) {
        m_sourceFailures = m_worker->m_sourceFailures;
        for (auto it = m_worker->m_sourceCooldown.cbegin(); it != m_worker->m_sourceCooldown.cend(); ++it)
            m_sourceCooldown[it.key()] = qMax(m_sourceCooldown.value(it.key()), it.value());
        m_worker->cancel();
    }
    emit changed();
    QTimer::singleShot(0, this, &AudioSources::startRecovery);
}
void AudioSources::cancelRecovery()
{
    m_recoveryQueue.clear();
    m_recoveryIds.clear();
    m_recoveryId = 0;
    m_recoveryPhase = Idle;
    if (m_worker)
        m_worker->cancel();
    m_recoveryStatus = "Автоподбор остановлен.";
    emit changed();
}
void AudioSources::downloadLink(const QString& text)
{
    const QUrl url(text.trimmed());
    if (url.scheme() != "https" || m_downloads->serviceForUrl(url.toString()).isEmpty()) {
        m_status = "Нужна HTTPS-ссылка на запись поддерживаемого сервиса или аудиофайл.";
        emit changed();
        return;
    }
    const auto source = m_downloads->serviceForUrl(url.toString());
    if (source == "Hitmo" || source == "Zaycev.net") {
        search(source);
        m_results = {QVariantMap{{"url", url.toString()}, {"title", m_target.value("title")}}};
        cancel();
        choose(0);
        return;
    }
    queue({{"url", url.toString()}});
}

void AudioSources::previewCandidate(const QVariantMap& candidate)
{
    cancel();
    m_preview = true;
    m_catalogMode = false;
    m_source = candidate.value("source").toString();
    m_candidate = candidate;
    m_results = {candidate};
    choose(0);
}
void AudioSources::resolveStream()
{
    if (!m_tools || !m_tools->runtimeReady()) {
        finish("Сначала подготовь инструменты загрузки в настройках библиотеки.");
        return;
    }
    m_busy = true;
    m_resolving = true;
    m_elapsed.start();
    m_status = "Подключаем поток из " + m_source + "…";
    emit changed();
    m_process = new QProcess(this);
    m_output.clear();
    m_searchError.clear();
    const int generation = m_generation;
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this] {
        if (m_process) {
            m_output += m_process->readAllStandardOutput();
            if (m_output.size() > 4 * 1024 * 1024)
                finish("Слишком большой ответ при получении потока.");
        }
    });
    connect(m_process, &QProcess::readyReadStandardError, this, [this] {
        if (m_process)
            m_searchError = (m_searchError + QString::fromUtf8(m_process->readAllStandardError())).right(8192);
    });
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart)
            finish("Не удалось запустить подключение к аудио.");
    });
    connect(m_process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, generation](int code, QProcess::ExitStatus exit) {
                if (generation != m_generation || !m_process)
                    return;
                m_output += m_process->readAllStandardOutput();
                if (code || exit != QProcess::NormalExit) {
                    const auto error = DownloaderErrors::humanizeDownloaderError(m_searchError);
                    finish(error.isEmpty() ? "Не удалось получить поток этого источника." : error);
                    return;
                }
                const auto info = QJsonDocument::fromJson(m_output).object();
                const QUrl url(info.value("url").toString());
                if (url.scheme() != "https" || !url.userInfo().isEmpty()) {
                    finish("Источник не предоставил поток аудио.");
                    return;
                }
                auto candidate = m_candidate;
                candidate["mediaUrl"] = url.toString();
                candidate["mediaVerified"] = true;
                candidate["streamHeaders"] = info.value("http_headers").toObject().toVariantMap();
                queue(candidate);
            });
    QStringList args{"--ignore-config",
                     "--no-playlist",
                     "--dump-single-json",
                     "--skip-download",
                     "-f",
                     "bestaudio[protocol=https]/bestaudio/best",
                     "--socket-timeout",
                     "10",
                     "--retries",
                     "1"};
    args.append(m_tools->jsRuntimeArguments());
    args << "--" << m_candidate.value("url").toString();
    m_timer.start();
    m_process->start(m_tools->ytDlpExecutable(), args);
}
