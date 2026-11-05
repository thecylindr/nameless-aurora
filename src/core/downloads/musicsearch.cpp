#include "musicsearch.h"
#include "audiosources.h"
#include "../auth/webloginmanager.h"
#include "../librarymodel.h"
#include "../downloadmanager.h"
#include <QSettings>
#include <QUrl>
#include <QDesktopServices>

// Реализация поиска музыки: управляет воркерами источников, объединяет результаты,
// обрабатывает прослушивание (VK и онлайн-потоки) и скачивание.
MusicSearch::MusicSearch(LibraryModel* library, DownloadManager* downloads, DependencyManager* tools,
                         WebLoginManager* web, QObject* parent)
    : QObject(parent), m_library(library), m_downloads(downloads), m_web(web)
{
    m_sources = QSettings().value("discovery/sources", QStringList{"YouTube", "Zaycev.net"}).toStringList();
    m_sources.removeDuplicates();
    for (auto it = m_sources.begin(); it != m_sources.end();)
        if (!QStringList{"YouTube", "Zaycev.net", "VK"}.contains(*it))
            it = m_sources.erase(it);
        else
            ++it;
    m_taste = QSettings().value("discovery/taste").toString().left(200);
    m_audition = new AudioSources(library, downloads, tools, this, {}, true);
    connect(m_audition, &AudioSources::previewResolved, this,
            [this](const QVariantMap& candidate, const QString& error) {
                if (m_auditionKey.isEmpty())
                    return;
                m_auditionTimer.stop();
                m_auditionKey.clear();
                m_status = error.isEmpty() ? "Слушаем онлайн · скачивание доступно отдельно" : error;
                emit changed();
                if (error.isEmpty())
                    emit playRequested(candidate);
            });
    m_auditionTimer.setSingleShot(true);
    m_auditionTimer.setInterval(45000);
    connect(&m_auditionTimer, &QTimer::timeout, this, [this] {
        cancelAudition();
        m_status = "Сервис не предоставил поток за 45 секунд. Выбери другую запись.";
        emit changed();
    });
    connect(web, &WebLoginManager::mediaReady, this, [this](int request, const QUrl& url, const QString& error) {
        if (request != m_auditionRequest || m_auditionKey.isEmpty())
            return;
        auto candidate = m_auditionCandidate;
        candidate["mediaUrl"] = url.toString();
        candidate["userAgent"] = m_web->serviceUserAgent();
        m_auditionRequest = 0;
        m_auditionKey.clear();
        m_auditionTimer.stop();
        m_status = error.isEmpty() ? "Слушаем VK онлайн" : error;
        emit changed();
        if (error.isEmpty() && url.scheme() == "https")
            emit playRequested(candidate);
    });
    for (const auto& source : QStringList{"YouTube", "Zaycev.net"}) {
        auto* worker = new AudioSources(library, downloads, tools, this, {}, true);
        m_workers[source] = worker;
        connect(worker, &AudioSources::operationFinished, this, [this, source, worker] {
            if (worker->busy())
                return;
            if (m_downloadSource == source) {
                m_status = worker->status();
                m_downloadKey.clear();
                m_downloadSource.clear();
                emit changed();
            } else if (m_pending.contains(source))
                receive(source, worker->results(), worker->status());
        });
        connect(worker, &AudioSources::catalogQueued, this, [this](int id) {
            if (!m_downloadKey.isEmpty())
                m_saved[m_downloadKey] = id;
            emit changed();
        });
    }
    connect(web, &WebLoginManager::searchReady, this, [this](const QVector<Track>& tracks, const QString& error) {
        if (!m_pending.contains("VK"))
            return;
        QVariantList result;
        for (const auto& track : tracks) {
            auto item = track.webData;
            item["title"] = track.title;
            item["artist"] = track.artist;
            item["album"] = track.album;
            item["url"] = track.sourceUrl;
            item["coverUrl"] = track.coverUrl;
            item["durationMs"] = track.duration;
            item["mediaUrl"] = track.mediaUrl;
            item["source"] = "VK";
            result.append(item);
        }
        receive("VK", result, error);
    });
}
void MusicSearch::setEnabledSources(const QStringList& sources)
{
    QStringList next;
    for (const auto& s : sources)
        if (QStringList{"YouTube", "Zaycev.net", "VK"}.contains(s) && !next.contains(s))
            next.append(s);
    if (next == m_sources)
        return;
    m_sources = next;
    QSettings().setValue("discovery/sources", next);
    emit preferencesChanged();
}
void MusicSearch::setTaste(const QString& taste)
{
    const auto next = taste.trimmed().left(200);
    if (next == m_taste)
        return;
    m_taste = next;
    QSettings().setValue("discovery/taste", next);
    emit preferencesChanged();
}
void MusicSearch::cancel()
{
    cancelAudition();
    const bool vk = m_pending.contains("VK");
    m_pending.clear();
    m_downloadKey.clear();
    m_downloadSource.clear();
    for (auto* worker : m_workers)
        worker->cancel();
    if (vk)
        m_web->cancelSearch();
    m_status = "Поиск остановлен";
    emit changed();
}
void MusicSearch::cancelAudition()
{
    const int request = m_auditionRequest;
    m_auditionRequest = 0;
    m_auditionKey.clear();
    m_auditionTimer.stop();
    if (m_audition)
        m_audition->cancel();
    if (request)
        m_web->cancelMedia(request);
    emit changed();
}
void MusicSearch::listen(const QString& key)
{
    cancelAudition();
    const int id = m_saved.value(key);
    if (id > 0 && m_library->trackDetails(id).value("downloaded").toBool()) {
        emit playLibraryRequested(id);
        return;
    }
    QVariantMap candidate;
    for (const auto& item : m_results)
        if (item.toMap().value("key").toString() == key) {
            candidate = item.toMap();
            break;
        }
    if (candidate.isEmpty())
        return;
    m_auditionKey = key;
    m_auditionCandidate = candidate;
    m_auditionTimer.start();
    m_status = "Подключаем прослушивание…";
    emit changed();
    if (candidate.value("source").toString() == "VK") {
        // Для VK используем отрицательный идентификатор, чтобы не пересекаться с обычными загрузками.
        m_auditionRequest = -1000000 - (++m_auditionSerial);
        m_web->resolveMedia(m_auditionRequest, candidate);
    } else
        m_audition->previewCandidate(candidate);
}
void MusicSearch::search(const QString& query)
{
    if (!query.trimmed().isEmpty())
        begin(query.trimmed().left(250), false);
}
void MusicSearch::discover()
{
    QString seed = m_taste;
    if (seed.isEmpty()) {
        const auto tracks = m_library->recommendations(1);
        if (!tracks.isEmpty())
            seed = tracks.first().toMap().value("artist").toString();
    }
    if (seed.isEmpty())
        seed = "новая музыка";
    begin(seed, true);
}
void MusicSearch::begin(const QString& query, bool recommendations)
{
    cancel();
    m_query = query;
    m_batches.clear();
    m_results.clear();
    m_states.clear();
    if (m_sources.isEmpty()) {
        m_status = "Выбери хотя бы один источник.";
        emit changed();
        return;
    }
    for (const auto& source : m_sources) {
        m_pending.insert(source);
        m_states[source] = "Ищем…";
    }
    m_status = recommendations ? "Собираем идеи по твоему вкусу…" : "Ищем музыку…";
    emit changed();
    // Все флаги ожидания выставляем заранее, пока воркер не успел вернуть результат из кэша синхронно.
    for (const auto& source : m_sources) {
        if (source == "VK")
            m_web->searchMusic(query, recommendations);
        else
            m_workers.value(source)->searchCatalog(source, query);
    }
}
void MusicSearch::receive(const QString& source, const QVariantList& results, const QString& status)
{
    if (!m_pending.remove(source))
        return;
    m_batches[source] = results;
    m_states[source] = results.isEmpty() ? (status.isEmpty() ? "Ничего не найдено" : status)
                                         : QString("%1 вариантов").arg(results.size());
    rebuild();
    m_status = busy() ? QString("Найдено %1 · другие источники ещё ищут…").arg(m_results.size())
               : m_results.isEmpty()
                   ? "Ничего не найдено. Измени запрос или источник."
                   : QString("Найдено %1 записей · выбери версию и добавь в плеер").arg(m_results.size());
    emit changed();
}
void MusicSearch::rebuild()
{
    m_results.clear();
    QSet<QString> seen;
    for (const auto& track : m_library->repository().tracks())
        if (!track.sourceUrl.isEmpty())
            m_saved[track.sourceUrl] = track.id;
    // Чередуем результаты источников построчно, чтобы выдача была разнообразной.
    for (int row = 0; row < 36; ++row)
        for (const auto& source : m_sources) {
            const auto list = m_batches.value(source);
            if (row >= list.size())
                continue;
            auto item = list[row].toMap();
            const auto key = item.value("url").toString();
            const QUrl url(key);
            if (url.scheme() != "https" || !url.userInfo().isEmpty() || seen.contains(key) ||
                m_downloads->serviceForUrl(key) != source)
                continue;
            seen.insert(key);
            item["source"] = source;
            item["key"] = key;
            m_results.append(item);
        }
}
void MusicSearch::download(const QString& key)
{
    if (resolving())
        return;
    QVariantMap candidate;
    for (const auto& item : m_results)
        if (item.toMap().value("key").toString() == key) {
            candidate = item.toMap();
            break;
        }
    if (candidate.isEmpty())
        return;
    const QString source = candidate.value("source").toString();
    if (m_pending.contains(source)) {
        m_status = "Дождись завершения поиска этого источника.";
        emit changed();
        return;
    }
    if (source == "VK") {
        Track track;
        track.title = candidate.value("title").toString();
        track.artist = candidate.value("artist").toString();
        track.album = candidate.value("album").toString();
        track.source = "VK";
        track.sourceUrl = key;
        track.duration = candidate.value("durationMs").toLongLong();
        const int id = m_library->repository().saveTrack(track);
        if (id) {
            m_saved[key] = id;
            m_library->refresh();
            m_library->cacheCover(id, QUrl(candidate.value("coverUrl").toString()));
            m_status = m_downloads->enqueueAlternative(id, candidate, {}) ? "Трек VK добавлен в очередь."
                                                                          : "Трек уже в плеере или в очереди.";
        } else
            m_status = "Не удалось сохранить трек.";
        emit changed();
        return;
    }
    auto* worker = m_workers.value(source);
    if (!worker)
        return;
    m_downloadKey = key;
    m_downloadSource = source;
    m_status = "Получаем аудио из " + source + "…";
    emit changed();
    worker->downloadCandidate(candidate);
}
void MusicSearch::openInService(const QString& key)
{
    for (const auto& item : m_results)
        if (item.toMap().value("key").toString() == key) {
            QDesktopServices::openUrl(QUrl(key));
            break;
        }
}
