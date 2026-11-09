#include "homefeed.h"

#include "auth/oauthmanager.h"
#include "auth/webloginmanager.h"
#include "downloads/audiosources.h"
#include "librarymodel.h"
#include "playbackcontroller.h"
#include "web/servicepayload.h"
#include "web/mediasniffer.h"
#include <QDateTime>
#include <QFileInfo>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QSet>
#include <QSettings>
#include <QRegularExpression>
#include <QUrlQuery>
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace
{
QVariantMap normalizedPreferences(const QVariantMap& values)
{
    auto mode = values.value("discoveryMode", "balanced").toString();
    if (!QStringList{"balanced", "discovery", "familiar"}.contains(mode))
        mode = "balanced";
    auto source = values.value("waveSource", "auto").toString();
    if (!QStringList{"auto", "local", "yandex", "vk"}.contains(source))
        source = "auto";
    QVariantList excluded;
    for (const auto& entry : values.value("excludedPlaylists").toList()) {
        const int id = entry.toInt();
        if (id > 0 && !excluded.contains(id))
            excluded.append(id);
        if (excluded.size() >= 500)
            break;
    }
    return {{"discoveryMode", mode},
            {"waveSource", source},
            {"excludedPlaylists", excluded},
            {"onlyDownloaded", values.value("onlyDownloaded", false).toBool()},
            {"includeUnsorted", values.value("includeUnsorted", true).toBool()},
            {"showYandex", values.value("showYandex", true).toBool()},
            {"showVk", values.value("showVk", true).toBool()}};
}

quint32 rotationSeed(const QString& value)
{
    quint32 hash = 2166136261u;
    for (const auto ch : value)
        hash = (hash ^ ch.unicode()) * 16777619u;
    hash ^= hash >> 16;
    hash *= 0x85ebca6bu;
    hash ^= hash >> 13;
    return hash;
}

QVariantMap describe(const Track& track)
{
    const bool downloaded = !track.filePath.isEmpty() && QFileInfo(track.filePath).isFile();
    // A missing source URL can still be resolved by title/artist in enabled catalogs.
    const bool online = !track.title.trimmed().isEmpty();
    auto item = track.webData;
    const QVariantMap details{
        {"trackId", track.id},
        {"title", track.title},
        {"artist", track.artist},
        {"album", track.album},
        {"coverPath", track.coverPath},
        {"source", track.source},
        {"downloaded", downloaded},
        {"libraryId", downloaded ? track.id : 0},
        {"libraryTrackId", track.id},
        {"url", track.sourceUrl.isEmpty() ? "aurora:track/" + QString::number(track.id) : track.sourceUrl},
        {"key", track.sourceUrl.isEmpty() ? "aurora:track/" + QString::number(track.id) : track.sourceUrl},
        {"coverUrl", track.coverUrl},
        {"durationMs", track.duration},
        {"mediaUrl", track.mediaUrl},
        {"canListen", downloaded || online}};
    for (auto it = details.cbegin(); it != details.cend(); ++it)
        item[it.key()] = it.value();
    return item;
}

QVariantMap makeMix(const QVector<Track>& tracks, const QString& title, const QString& subtitle, const QString& seed,
                    int limit, int playlistId, const QString& mode)
{
    struct Ranked
    {
        Track track;
        double score;
    };
    QVector<Ranked> ranked;
    for (const auto& track : tracks) {
        const double rotation =
            rotationSeed(seed + ":" + QString::number(track.id)) / double(std::numeric_limits<quint32>::max());
        // Новые и забытые записи конкурируют с любимыми, а не с накопленным счётчиком прослушиваний.
        const double novelty = mode == "discovery" ? 20.0 : mode == "familiar" ? 0.0 : 9.0;
        const double familiar = mode == "familiar" && track.playCount > 0 ? 12.0 : 0.0;
        const double score = rotation * 18.0 + (track.playCount == 0 ? novelty : familiar) +
                             track.rating * (mode == "familiar" ? 3.0 : 1.4) - std::min(6.0, track.skipCount * 1.5);
        ranked.append({track, score});
    }
    std::stable_sort(ranked.begin(), ranked.end(), [](const Ranked& a, const Ranked& b) {
        return a.score == b.score ? a.track.id < b.track.id : a.score > b.score;
    });

    QVariantList selected, ids, playable, covers;
    QMap<QString, int> sourceCounts;
    int onlineCount = 0;
    QHash<QString, int> artistCounts;
    QSet<int> seen;
    // Сначала даём место разным исполнителям, затем дополняем маленькие/авторские плейлисты.
    for (int pass = 0; pass < 2 && selected.size() < limit; ++pass) {
        for (const auto& item : ranked) {
            const auto& track = item.track;
            const auto artist = track.artist.toCaseFolded().trimmed();
            if (seen.contains(track.id) || (pass == 0 && artistCounts.value(artist) >= 2))
                continue;
            const auto detail = describe(track);
            selected.append(detail);
            const auto source = track.source.trimmed();
            ++sourceCounts[source.isEmpty() ? QStringLiteral("Локальные файлы") : source];
            ids.append(track.id);
            if (detail.value("downloaded").toBool())
                playable.append(track.id);
            else if (detail.value("canListen").toBool())
                ++onlineCount;
            if (covers.size() < 4)
                covers.append(QVariantMap{{"path", track.coverPath}, {"seed", track.album + track.title}});
            ++artistCounts[artist];
            seen.insert(track.id);
            if (selected.size() == limit)
                break;
        }
    }
    QVariantList platforms;
    for (auto it = sourceCounts.cbegin(); it != sourceCounts.cend(); ++it)
        platforms.append(QVariantMap{{"name", it.key()}, {"count", it.value()}});
    return {{"platforms", platforms},
            {"origin", "Из твоей библиотеки"},
            {"description", playlistId > 0 ? "Микс Aurora из плейлиста. Порядок обновляется по выбранному периоду."
                                           : "Микс Aurora из выбранных плейлистов и несортированных треков. Это музыка "
                                             "из твоей коллекции, а не новые рекомендации сервиса."},
            {"title", title},
            {"subtitle", subtitle},
            {"playlistId", playlistId},
            {"tracks", selected},
            {"trackIds", ids},
            {"playableIds", playable},
            {"covers", covers},
            {"onlineCount", onlineCount},
            {"count", selected.size()}};
}
QString mixTitle(const QString& name, const QVariantList& tracks, int period)
{
    const auto clean = name.simplified();
    if (QStringList{"мне нравится", "любимое", "избранное", "liked songs"}.contains(clean.toCaseFolded()))
        return QStringList{"Любимое сегодня", "Любимое на три дня", "Любимое на неделю"}[period];
    const QRegularExpression generic(
        "^(главная|импортированн.*|плейлист(?: \\d+)?|playlist(?: \\d+)?|imported.*|моя музыка|моя коллекция)$",
        QRegularExpression::CaseInsensitiveOption);
    if (clean.isEmpty() || generic.match(clean).hasMatch()) {
        const auto artist = tracks.isEmpty() ? QString() : tracks.first().toMap().value("artist").toString().trimmed();
        return artist.isEmpty() ? "Личный микс" : "Микс · " + artist;
    }
    return clean;
}
} // namespace

QVariantList HomeFeed::buildSections(LibraryModel* library, const QDate& date, const QVariantMap& options)
{
    const auto all = library->repository().tracks();
    if (all.isEmpty())
        return {};

    // Читаем каждый плейлист один раз; смена вкладки в QML не обращается к базе.
    struct Playlist
    {
        int id;
        QString title;
        QVector<Track> tracks;
    };
    QVector<Playlist> playlists;
    QSet<int> membership, included;
    const auto excluded = options.value("excludedPlaylists").toList();
    const bool onlyDownloaded = options.value("onlyDownloaded", false).toBool();
    const auto eligible = [onlyDownloaded](const Track& track) {
        return !onlyDownloaded || (!track.filePath.isEmpty() && QFileInfo(track.filePath).isFile());
    };
    for (const auto& entry : library->playlists()) {
        const auto playlist = entry.toMap();
        const int id = playlist.value("id").toInt();
        auto tracks = library->repository().tracks({}, id);
        for (const auto& track : tracks)
            membership.insert(track.id);
        if (excluded.contains(id))
            continue;
        tracks.erase(std::remove_if(tracks.begin(), tracks.end(),
                                    [&](const Track& track) {
                                        return !eligible(track);
                                    }),
                     tracks.end());
        for (const auto& track : tracks)
            included.insert(track.id);
        if (!tracks.isEmpty())
            playlists.append({id, playlist.value("name").toString(), std::move(tracks)});
    }
    QVector<Track> collection;
    for (const auto& track : all) {
        const bool unsorted = !membership.contains(track.id) && options.value("includeUnsorted", true).toBool();
        if (eligible(track) && (included.contains(track.id) || unsorted))
            collection.append(track);
    }
    if (collection.isEmpty())
        return {};
    const QString mode = options.value("discoveryMode", "balanced").toString();

    const QDate monday = date.addDays(1 - date.dayOfWeek());
    const QList<qint64> periods{date.toJulianDay(), date.toJulianDay() / 3, monday.toJulianDay()};
    const QStringList labels{"На сегодня", "На 3 дня", "На этой неделе"};
    const QStringList captions{"Свежий взгляд на твою коллекцию", "Больше музыки для ближайших дней",
                               "Большая подборка из твоих плейлистов"};
    const QList<int> limits{18, 36, 60};
    QVariantList sections;
    for (int period = 0; period < periods.size(); ++period) {
        const auto seed = QString::number(period) + ":" + QString::number(periods[period]);
        QVariantList mixes;
        mixes.append(makeMix(collection,
                             period == 0   ? "Твоя коллекция сегодня"
                             : period == 1 ? "Три дня музыки"
                                           : "Твоя неделя",
                             "Из выбранных плейлистов и коллекции", seed, limits[period], 0, mode));
        // Ротация затрагивает и набор плейлистов на полке, если их много.
        auto ordered = playlists;
        std::stable_sort(ordered.begin(), ordered.end(), [&seed](const Playlist& a, const Playlist& b) {
            return rotationSeed(seed + ":playlist:" + QString::number(a.id)) <
                   rotationSeed(seed + ":playlist:" + QString::number(b.id));
        });
        for (const auto& playlist : ordered) {
            auto mix = makeMix(playlist.tracks, playlist.title, "Из «" + playlist.title + "»",
                               seed + ":list:" + QString::number(playlist.id), limits[period], playlist.id, mode);
            mix["title"] = mixTitle(playlist.title, mix.value("tracks").toList(), period);
            mix["playlistName"] = playlist.title;
            mixes.append(mix);
            if (mixes.size() == 7)
                break;
        }
        sections.append(QVariantMap{{"title", labels[period]},
                                    {"subtitle", captions[period]},
                                    {"mixes", mixes},
                                    {"date", date.toString(Qt::ISODate)}});
    }
    return sections;
}

HomeFeed::HomeFeed(LibraryModel* library, OAuthManager* accounts, WebLoginManager* web, PlaybackController* player,
                   QObject* parent, QNetworkAccessManager* transport, AudioSources* preview)
    : QObject(parent), m_library(library), m_accounts(accounts), m_web(web), m_player(player),
      m_network(transport ? transport : new QNetworkAccessManager(this)), m_preview(preview),
      m_preferences(normalizedPreferences(QSettings().value("home/recommendations").toMap()))
{
    m_refreshTimer.setSingleShot(true);
    m_refreshTimer.setInterval(180);
    connect(library, &LibraryModel::libraryChanged, &m_refreshTimer, qOverload<>(&QTimer::start));
    connect(&m_refreshTimer, &QTimer::timeout, this, &HomeFeed::refresh);
    connect(accounts, &OAuthManager::accountsChanged, this, &HomeFeed::accountChanged);
    m_webConnected = web->yandexLoggedIn();
    m_vkWebConnected = web->vkLoggedIn();
    connect(web, &WebLoginManager::sessionsChanged, this, [this] {
        m_streamCache.clear();
        const bool connected = m_web->yandexLoggedIn();
        if (m_webConnected != connected) {
            m_webConnected = connected;
            accountChanged();
        } else
            emit yandexChanged();
        const bool vk = m_web->vkLoggedIn();
        if (m_vkWebConnected != vk) {
            m_vkWebConnected = vk;
            m_vkBusy = false;
            m_vkWaiting = false;
            m_playVkWhenReady = false;
            m_vkTracks.clear();
            m_vkFetchedAt = 0;
            m_web->cancelVkRecommendations();
            if (!vk && WebLoginManager::serviceForHost(QUrl(m_candidate.value("url").toString()).host()) == "vk")
                m_player->stop();
            m_vkStatus.clear();
            if (vk && m_preferences.value("showVk").toBool())
                QTimer::singleShot(0, this, [this] {
                    refreshVk(true);
                });
        }
        emit vkChanged();
    });
    connect(web, &WebLoginManager::waveReady, this, &HomeFeed::receiveYandex);
    connect(web, &WebLoginManager::vkRecommendationsReady, this, &HomeFeed::receiveVk);
    connect(web, &WebLoginManager::busyChanged, this, [this] {
        if (!m_web->busy())
            QTimer::singleShot(0, this, &HomeFeed::pumpProviders);
    });
    connect(web, &WebLoginManager::loginDone, this, [this](const QString& service, bool ok, const QString&) {
        const auto slug = WebLoginManager::normalizeService(service);
        if (ok) {
            if (!m_candidate.isEmpty() && m_resolvingId != 0 &&
                WebLoginManager::serviceForHost(QUrl(m_candidate.value("url").toString()).host()) == slug)
                resolveStream(m_candidate);
            else if (slug == "yandex")
                refreshYandex(true);
            else if (slug == "vk")
                refreshVk(true);
        }
    });
    connect(player, &PlaybackController::streamQueueExhausted, this, [this] {
        if (m_queueProvider == "yandex") {
            m_playWhenReady = true;
            refreshYandex(true);
        } else {
            m_player->stop();
        }
    });
    connect(player, &PlaybackController::streamRequested, this, &HomeFeed::resolveStream);
    connect(web, &WebLoginManager::mediaReady, this, [this](int request, const QUrl& url, const QString& error) {
        if (!m_mediaRequest || request != m_mediaRequest)
            return;
        auto candidate = m_candidate;
        candidate["mediaUrl"] = url.toString();
        candidate["userAgent"] = m_web->serviceUserAgent();
        finishStream(candidate, error);
    });
    if (m_preview) {
        m_lookup = m_preview->createWorker(this);
        connect(m_lookup, &AudioSources::resultsChanged, this, &HomeFeed::receiveAlternatives);
        connect(m_lookup, &AudioSources::operationFinished, this, &HomeFeed::receiveAlternatives);
        connect(m_preview, &AudioSources::previewResolved, this,
                [this](const QVariantMap& candidate, const QString& error) {
                    if (m_previewPending)
                        finishStream(candidate, error);
                });
    }
    m_totalResolveTimer.setParent(this);
    m_totalResolveTimer.setObjectName("onlineResolutionDeadline");
    m_totalResolveTimer.setSingleShot(true);
    m_totalResolveTimer.setInterval(45000);
    connect(&m_totalResolveTimer, &QTimer::timeout, this, [this] {
        const bool ownStream = m_player->streaming() && m_player->streamKey() == m_candidate.value("key").toString();
        const auto cached = m_streamCache.take(m_candidate.value("key").toString()).second;
        if (m_preview)
            m_preview->invalidatePreview(m_selectedAlternative.value("url", cached.value("resolvedUrl", m_candidate.value("url"))).toString());
        cancelResolution();
        if (ownStream)
            m_player->stop();
        m_playbackStatus = "Не удалось подключить трек за 45 секунд. Повтори или выбери источник вручную.";
        emit playbackChanged();
    });
    m_resolveTimer.setSingleShot(true);
    m_resolveTimer.setInterval(15000);
    connect(&m_resolveTimer, &QTimer::timeout, this, [this] {
        failAttempt("Источник не предоставил поток за 15 секунд.");
    });
    m_sourceGraceTimer.setSingleShot(true);
    m_sourceGraceTimer.setInterval(1500);
    connect(&m_sourceGraceTimer, &QTimer::timeout, this, [this] {
        m_fallbackAllowed = true;
        tryAlternative();
    });
    m_bufferTimer.setSingleShot(true);
    m_bufferTimer.setInterval(20000);
    connect(&m_bufferTimer, &QTimer::timeout, this, [this] {
        failAttempt("Поток не начал воспроизведение или перестал отвечать.");
    });
    connect(player, &PlaybackController::positionChanged, this, &HomeFeed::playbackStarted);
    connect(player, &PlaybackController::durationChanged, this, [this] {
        if (m_candidate.isEmpty() || m_player->streamKey() != m_candidate.value("key").toString() || m_retryScheduled)
            return;
        const auto expected = m_candidate.value("durationMs").toLongLong();
        const auto actual = m_player->duration();
        if (expected <= 0 || actual <= 0 || qAbs(expected - actual) <= qMax<qint64>(10000, expected / 10))
            return;
        const int generation = m_resolutionGeneration;
        m_retryScheduled = true;
        QTimer::singleShot(0, this, [this, generation] {
            if (generation != m_resolutionGeneration)
                return;
            m_retryScheduled = false;
            failAttempt("Источник отдал фрагмент или запись с другой длительностью.");
        });
    });
    connect(player, &PlaybackController::streamLoadingChanged, this, [this] {
        if (!m_candidate.isEmpty() && m_player->streamKey() == m_candidate.value("key").toString() &&
            m_player->streamLoading() && !m_bufferTimer.isActive())
            m_bufferTimer.start();
    });
    connect(player, &PlaybackController::lastErrorChanged, this, [this] {
        if (m_candidate.isEmpty() || m_player->lastError().isEmpty() || m_startingStream || m_retryScheduled)
            return;
        if (!m_playbackAttempt && m_player->streamKey() != m_candidate.value("key").toString())
            return;
        const int generation = m_resolutionGeneration;
        const auto error = m_player->lastError();
        m_retryScheduled = true;
        QTimer::singleShot(0, this, [this, generation, error] {
            if (generation != m_resolutionGeneration)
                return;
            m_retryScheduled = false;
            failAttempt(error);
        });
    });
    auto* midnight = new QTimer(this);
    midnight->setInterval(60000);
    connect(midnight, &QTimer::timeout, this, [this] {
        if (m_date != QDate::currentDate())
            refresh();
    });
    midnight->start();
    refresh();
}

bool HomeFeed::yandexConnected() const
{
    return m_accounts->yandexLoggedIn() || m_web->yandexLoggedIn();
}

void HomeFeed::refresh()
{
    m_date = QDate::currentDate();
    m_sections = buildSections(m_library, m_date, m_preferences);
    emit sectionsChanged();
}

void HomeFeed::accountChanged()
{
    ++m_generation;
    m_busy = false;
    m_yandexTracks.clear();
    m_recentYandex.clear();
    m_fetchedAt = 0;
    m_playWhenReady = false;
    m_yandexWaiting = false;
    m_web->cancelWave();
    if (!yandexConnected() &&
        WebLoginManager::serviceForHost(QUrl(m_candidate.value("url").toString()).host()) == "yandex")
        m_player->stop();
    m_status.clear();
    emit yandexChanged();
}

void HomeFeed::refreshYandex(bool force)
{
    if (m_busy)
        return;
    if (!yandexConnected()) {
        m_playWhenReady = false;
        m_status = "Подключи Яндекс Музыку, чтобы получать рекомендации своего аккаунта.";
        emit yandexChanged();
        return;
    }
    if (!force && !m_yandexTracks.isEmpty() && QDateTime::currentSecsSinceEpoch() - m_fetchedAt < 1800)
        return;
    m_busy = true;
    m_status = "Собираем рекомендации Яндекса…";
    emit yandexChanged();
    fetchYandex();
}

void HomeFeed::fetchYandex()
{
    if (m_web->yandexLoggedIn()) {
        m_yandexWaiting = true;
        pumpProviders();
        return;
    }
    const bool playWhenReady = m_playWhenReady;
    const QString token = m_accounts->yandexToken();
    // Обновление или истечение OAuth-токена тоже вызывает accountsChanged().
    if (!m_busy) {
        m_busy = true;
        m_playWhenReady = playWhenReady;
    }
    if (token.isEmpty()) {
        m_yandexWaiting = true;
        pumpProviders();
        return;
    }
    QNetworkRequest request(QUrl("https://api.music.yandex.net/rotor/station/user:onyourwave/tracks?settings2=true"));
    if (!m_recentYandex.isEmpty()) {
        auto url = request.url();
        QUrlQuery query(url);
        query.addQueryItem("queue", m_recentYandex.join(','));
        url.setQuery(query);
        request.setUrl(url);
    }
    request.setRawHeader("Authorization", "OAuth " + token.toUtf8());
    request.setRawHeader("User-Agent", "AuroraPlayer/4.0");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    request.setTransferTimeout(12000);
    auto* reply = m_network->get(request);
    reply->setReadBufferSize(2 * 1024 * 1024 + 1);
    const int generation = m_generation;
    connect(reply, &QNetworkReply::readyRead, reply, [reply] {
        if (reply->bytesAvailable() > 2 * 1024 * 1024)
            reply->abort();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, generation] {
        reply->deleteLater();
        if (generation != m_generation)
            return;
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if ((status == 401 || status == 403) && m_web->yandexLoggedIn()) {
            m_yandexWaiting = true;
            pumpProviders();
            return;
        }
        if (reply->error() != QNetworkReply::NoError || status != 200) {
            receiveYandex({}, status == 401 || status == 403
                                  ? "Яндекс отклонил сессию. Войди в аккаунт заново."
                                  : "Не удалось получить волну Яндекса. Проверь соединение и повтори.");
            return;
        }
        const auto data = reply->readAll();
        const auto document = QJsonDocument::fromJson(data);
        receiveYandex(ServicePayload::yandexWave(document.object()), {});
    });
}

void HomeFeed::receiveYandex(const QVector<Track>& tracks, const QString& error)
{
    if (!m_busy)
        return;
    m_busy = false;
    m_yandexWaiting = false;
    if (error.isEmpty() && !tracks.isEmpty()) {
        m_yandexTracks.clear();
        for (const auto& track : tracks) {
            auto item = describe(track);
            item["url"] = track.sourceUrl;
            item["coverUrl"] = track.coverUrl;
            item["coverPath"] = track.coverUrl;
            item["duration"] = track.duration;
            item["durationMs"] = track.duration;
            item["source"] = "Яндекс Музыка";
            m_yandexTracks.append(item);
            m_recentYandex.append(QUrl(track.sourceUrl).path().section('/', -1));
        }
        while (m_recentYandex.size() > 50)
            m_recentYandex.removeFirst();
        m_fetchedAt = QDateTime::currentSecsSinceEpoch();
        m_status = "Рекомендации твоего аккаунта · Яндекс Музыка";
    } else {
        m_status = error.isEmpty() ? "Яндекс пока не вернул треки для волны. Попробуй обновить позже." : error;
    }
    const bool play = std::exchange(m_playWhenReady, false);
    emit yandexChanged();
    if (play && error.isEmpty() && !tracks.isEmpty())
        playYandex();
    QTimer::singleShot(0, this, &HomeFeed::pumpProviders);
}

void HomeFeed::setPreferences(const QVariantMap& preferences)
{
    auto merged = m_preferences;
    for (auto it = preferences.cbegin(); it != preferences.cend(); ++it)
        merged[it.key()] = it.value();
    const auto next = normalizedPreferences(merged);
    if (next == m_preferences)
        return;
    const bool enableVk = !m_preferences.value("showVk").toBool() && next.value("showVk").toBool();
    const bool enableYandex = !m_preferences.value("showYandex").toBool() && next.value("showYandex").toBool();
    m_preferences = next;
    if (enableVk && vkConnected())
        QTimer::singleShot(0, this, [this] {
            refreshVk();
        });
    if (enableYandex && yandexConnected())
        QTimer::singleShot(0, this, [this] {
            refreshYandex();
        });
    QSettings().setValue("home/recommendations", next);
    emit preferencesChanged();
    m_refreshTimer.start();
}

bool HomeFeed::vkConnected() const
{
    return m_web->vkLoggedIn();
}

void HomeFeed::pumpProviders()
{
    if (m_web->busy())
        return;
    if (m_yandexWaiting && m_busy) {
        m_yandexWaiting = false;
        m_web->requestWave(m_recentYandex);
    } else if (m_vkWaiting && m_vkBusy) {
        m_vkWaiting = false;
        m_web->requestVkRecommendations();
    }
}

void HomeFeed::refreshVk(bool force)
{
    if (m_vkBusy)
        return;
    if (!vkConnected()) {
        m_vkStatus = "Подключи аккаунт VK, чтобы получать свою музыку и рекомендации.";
        m_playVkWhenReady = false;
        emit vkChanged();
        return;
    }
    if (!force && !m_vkTracks.isEmpty() && QDateTime::currentSecsSinceEpoch() - m_vkFetchedAt < 1800)
        return;
    m_vkBusy = true;
    m_vkWaiting = true;
    m_vkStatus = "Собираем рекомендации твоей VK Музыки…";
    emit vkChanged();
    pumpProviders();
}

void HomeFeed::receiveVk(const QVector<Track>& tracks, const QString& error)
{
    if (!m_vkBusy)
        return;
    m_vkBusy = false;
    m_vkWaiting = false;
    if (error.isEmpty() && !tracks.isEmpty()) {
        m_vkTracks.clear();
        QSet<QString> seen;
        for (const auto& track : tracks) {
            const QUrl url(track.sourceUrl);
            if (WebLoginManager::serviceForHost(url.host()) != "vk" || url.scheme() != "https" ||
                !url.userInfo().isEmpty() || track.title.trimmed().isEmpty() || seen.contains(track.sourceUrl))
                continue;
            seen.insert(track.sourceUrl);
            auto item = describe(track);
            item["coverPath"] = track.coverUrl;
            item["source"] = "VK";
            m_vkTracks.append(item);
            if (m_vkTracks.size() >= 200)
                break;
        }
        m_vkFetchedAt = QDateTime::currentSecsSinceEpoch();
    }
    m_vkStatus = !error.isEmpty()       ? error
                 : m_vkTracks.isEmpty() ? "VK пока не вернул рекомендации для этого аккаунта. Попробуй обновить позже."
                                        : "Персональные рекомендации твоего аккаунта VK";
    const bool play = std::exchange(m_playVkWhenReady, false);
    emit vkChanged();
    if (play && error.isEmpty() && !m_vkTracks.isEmpty())
        playVk();
    QTimer::singleShot(0, this, &HomeFeed::pumpProviders);
}

void HomeFeed::startQueue(const QVariantList& tracks, int index, const QString& provider)
{
    if (index < 0 || index >= tracks.size())
        return;
    m_queueProvider = provider;
    m_player->playStreamQueue(tracks, index);
}

void HomeFeed::playMix(const QVariantMap& mix, int trackId)
{
    QVariantList tracks;
    int index = 0;
    for (const auto& value : mix.value("tracks").toList()) {
        const int id = value.toMap().value("trackId").toInt();
        const auto track = m_library->trackById(id);
        if (!track.id)
            continue;
        const auto item = describe(track);
        if (!item.value("canListen").toBool())
            continue;
        if (id == trackId)
            index = tracks.size();
        tracks.append(item);
    }
    if (trackId && !describe(m_library->trackById(trackId)).value("canListen").toBool()) {
        findSource(trackId);
        return;
    }
    if (!tracks.isEmpty())
        startQueue(tracks, index, "mix");
    else if (!mix.value("trackIds").toList().isEmpty())
        findSource(mix.value("trackIds").toList().first().toInt());
}

void HomeFeed::playTrack(int trackId)
{
    const auto track = m_library->trackById(trackId);
    if (!track.id)
        return;
    const auto item = describe(track);
    const bool current = m_player->currentTrackId() == trackId ||
                         (!track.sourceUrl.isEmpty() && m_player->streamKey() == track.sourceUrl);
    if (current && m_resolvingId != trackId && m_player->lastError().isEmpty()) {
        m_player->toggle();
        return;
    }
    if (!item.value("canListen").toBool()) {
        findSource(trackId);
        return;
    }
    startQueue({item}, 0, "single");
}

void HomeFeed::playLibraryTrack(int trackId)
{
    const auto track = m_library->trackById(trackId);
    if (!track.id)
        return;
    const auto key = describe(track).value("key").toString();
    if ((m_player->streamKey() == key || m_player->currentTrackId() == trackId) && m_resolvingId == 0 &&
        m_player->lastError().isEmpty()) {
        m_player->toggle();
        return;
    }
    QVariantList tracks;
    int index = -1;
    for (int row = 0; row < m_library->rowCount(); ++row) {
        const int id = m_library->data(m_library->index(row, 0), LibraryModel::IdRole).toInt();
        const auto item = describe(m_library->trackById(id));
        if (!item.value("canListen").toBool())
            continue;
        if (id == trackId)
            index = tracks.size();
        tracks.append(item);
    }
    if (index >= 0)
        startQueue(tracks, index, "library");
    else
        startQueue({describe(track)}, 0, "single");
}

void HomeFeed::findSource(int trackId)
{
    if (m_library->trackById(trackId).id)
        emit sourceRequested(trackId);
}

void HomeFeed::playWave()
{
    QString source = m_preferences.value("waveSource").toString();
    if (source == "auto")
        source = yandexConnected() && m_preferences.value("showYandex").toBool() ? "yandex"
                 : vkConnected() && m_preferences.value("showVk").toBool()       ? "vk"
                                                                                 : "local";
    if (source == "yandex") {
        if (!m_yandexTracks.isEmpty())
            playYandex();
        else {
            m_playWhenReady = true;
            refreshYandex();
        }
    } else if (source == "vk") {
        if (!m_vkTracks.isEmpty())
            playVk();
        else {
            m_playVkWhenReady = true;
            refreshVk();
        }
    } else if (!m_sections.isEmpty()) {
        playMix(m_sections.first().toMap().value("mixes").toList().first().toMap());
    }
}

void HomeFeed::playYandex(int index)
{
    startQueue(m_yandexTracks, index, "yandex");
}

void HomeFeed::playVk(int index)
{
    startQueue(m_vkTracks, index, "vk");
}

void HomeFeed::cancelResolution()
{
    ++m_resolutionGeneration;
    m_totalResolveTimer.stop();
    m_resolveTimer.stop();
    m_sourceGraceTimer.stop();
    m_bufferTimer.stop();
    const int previous = std::exchange(m_mediaRequest, 0);
    if (previous)
        m_web->cancelMedia(previous);
    m_previewPending = false;
    if (m_preview)
        m_preview->cancel();
    if (m_lookup)
        m_lookup->cancel();
    m_lookupStarted = false;
    m_fallbackAllowed = false;
    m_playbackAttempt = false;
    m_fallbackAttempt = false;
    m_retryScheduled = false;
    m_alternatives.clear();
    m_selectedAlternative.clear();
    m_attemptedUrls.clear();
    m_sourceAttempts.clear();
    m_resolutionError.clear();
    m_resolvingId = 0;
    m_resolvingUrl.clear();
}

void HomeFeed::startLookup()
{
    if (!m_lookup || m_lookupStarted || m_candidate.isEmpty())
        return;
    m_lookupStarted = true;
    m_lookup->searchMatches(m_candidate);
}

void HomeFeed::receiveAlternatives()
{
    if (!m_lookupStarted || m_candidate.isEmpty())
        return;
    QSet<QString> known;
    for (const auto& entry : std::as_const(m_alternatives))
        known.insert(entry.toMap().value("url").toString());
    for (const auto& entry : m_lookup->results()) {
        const auto url = entry.toMap().value("url").toString();
        if (!known.contains(url)) {
            m_alternatives.append(entry);
            known.insert(url);
        }
    }
    QTimer::singleShot(0, this, &HomeFeed::tryAlternative);
}

void HomeFeed::tryAlternative()
{
    if (m_candidate.isEmpty() || !m_fallbackAllowed || m_playbackAttempt || (m_previewPending && m_fallbackAttempt))
        return;
    int best = -1;
    int bestScore = -1;
    for (int index = 0; index < m_alternatives.size(); ++index) {
        const auto item = m_alternatives[index].toMap();
        const auto url = item.value("url").toString();
        const auto service = item.value("source").toString();
        if (m_attemptedUrls.contains(url) || m_sourceAttempts.value(service) >= 2 ||
            !AudioSources::automaticMatch(m_candidate, item))
            continue;
        const auto expected = m_candidate.value("durationMs").toLongLong();
        const auto duration = item.value("durationMs").toLongLong();
        const int durationBonus =
            expected > 0 && duration > 0 ? 2000 - int(qMin<qint64>(2000, qAbs(expected - duration) / 10)) : 0;
        const int score = AudioSources::matchScore(m_candidate, item) * 10000 + durationBonus +
                          (item.value("mediaVerified").toBool() ? 10 : 0);
        if (score > bestScore) {
            bestScore = score;
            best = index;
        }
    }
    if (best >= 0 && m_preview) {
        const int previous = std::exchange(m_mediaRequest, 0);
        if (previous)
            m_web->cancelMedia(previous);
        m_preview->cancel();
        m_sourceGraceTimer.stop();
        m_selectedAlternative = m_alternatives[best].toMap();
        m_attemptedUrls.insert(m_selectedAlternative.value("url").toString());
        ++m_sourceAttempts[m_selectedAlternative.value("source").toString()];
        m_previewPending = true;
        m_fallbackAttempt = true;
        m_resolveTimer.start();
        m_playbackStatus = QString("Подключаем «%1» · %2 · попытка %3")
                               .arg(m_candidate.value("title").toString(), m_selectedAlternative.value("source").toString())
                               .arg(m_attemptedUrls.size());
        emit playbackChanged();
        m_preview->previewCandidate(m_selectedAlternative);
        return;
    }
    if (m_mediaRequest || m_previewPending)
        return;
    if (m_lookup && m_lookup->busy()) {
        m_playbackStatus = "Ищем подходящую версию во включённых источниках…";
    } else {
        m_totalResolveTimer.stop();
        m_resolvingId = 0;
        m_resolvingUrl.clear();
        m_playbackStatus = "Не удалось включить «" + m_candidate.value("title").toString() +
                           "». Подходящий доступный поток не найден; можно выбрать аудио вручную.";
        if (!m_resolutionError.isEmpty())
            m_playbackStatus += " " + m_resolutionError;
    }
    emit playbackChanged();
}

void HomeFeed::failAttempt(const QString& error)
{
    if (m_candidate.isEmpty())
        return;
    if (!m_totalResolveTimer.isActive())
        m_totalResolveTimer.start();
    m_resolutionError = error;
    const auto cached = m_streamCache.take(m_candidate.value("key").toString()).second;
    const auto failedUrl = m_selectedAlternative.value("url", cached.value("resolvedUrl", m_candidate.value("url"))).toString();
    m_attemptedUrls.insert(failedUrl);
    if (m_preview)
        m_preview->invalidatePreview(failedUrl);
    if (m_player->streamKey() == m_candidate.value("key").toString())
        m_player->pause();
    m_resolvingId = m_candidate.value("libraryTrackId").toInt();
    m_resolvingUrl = m_candidate.value("url").toString();
    m_resolveTimer.stop();
    m_bufferTimer.stop();
    const int request = std::exchange(m_mediaRequest, 0);
    if (request)
        m_web->cancelMedia(request);
    m_previewPending = false;
    if (m_preview)
        m_preview->cancel();
    m_playbackAttempt = false;
    m_fallbackAllowed = true;
    startLookup();
    tryAlternative();
}

void HomeFeed::playbackStarted()
{
    if (m_startingStream || m_candidate.isEmpty() || !m_player->playing() || m_player->position() <= 0 ||
        m_player->streamKey() != m_candidate.value("key").toString())
        return;
    m_totalResolveTimer.stop();
    m_bufferTimer.stop();
    m_playbackAttempt = false;
    if (m_resolvingId == 0 && m_resolvingUrl.isEmpty())
        return;
    m_resolvingId = 0;
    m_resolvingUrl.clear();
    m_fallbackAllowed = false;
    if (m_lookup)
        m_lookup->cancel();
    m_lookupStarted = false;
    m_playbackStatus = "Слушаем · " + m_candidate.value("title").toString() +
                       (m_fallbackAttempt ? " · " + m_selectedAlternative.value("source").toString() : "");
    emit playbackChanged();
}

void HomeFeed::finishStream(const QVariantMap& resolved, const QString& error)
{
    if (m_candidate.isEmpty())
        return;
    const QUrl media(resolved.value("mediaUrl").toString());
    if (!error.isEmpty() || media.scheme() != "https" || !media.userInfo().isEmpty()) {
        failAttempt(error.isEmpty() ? "Источник не предоставил поток." : error);
        return;
    }
    m_resolveTimer.stop();
    m_sourceGraceTimer.stop();
    m_previewPending = false;
    const int request = std::exchange(m_mediaRequest, 0);
    if (request)
        m_web->cancelMedia(request);
    auto candidate = m_candidate;
    candidate["mediaUrl"] = media.toString();
    candidate["userAgent"] = resolved.value("userAgent", m_web->serviceUserAgent());
    candidate["streamHeaders"] = resolved.value("streamHeaders");
    candidate["streamReferer"] = resolved.value("streamReferer", resolved.value("url", candidate.value("url")));
    if (resolved.contains("resolvedUrl")) {
        candidate["source"] = resolved.value("source");
        candidate["resolvedUrl"] = resolved.value("resolvedUrl");
        candidate["originSource"] = resolved.value("originSource");
    }
    if (m_fallbackAttempt) {
        candidate["originSource"] = candidate.value("source");
        candidate["source"] = m_selectedAlternative.value("source");
        candidate["resolvedUrl"] = m_selectedAlternative.value("url");
        if (candidate.value("coverUrl").toString().isEmpty())
            candidate["coverUrl"] = resolved.value("coverUrl");
    }
    // Keep the original url/key/queueId and title: an alternative must not replace
    // the playlist identity or write substitute metadata into the library.
    if (m_streamCache.size() >= 64)
        m_streamCache.clear();
    if (!candidate.value("key").toString().isEmpty())
        m_streamCache[candidate.value("key").toString()] = {QDateTime::currentMSecsSinceEpoch(), candidate};
    m_playbackAttempt = true;
    m_startingStream = true;
    m_player->playStream(candidate);
    m_startingStream = false;
    if (!m_player->lastError().isEmpty()) {
        failAttempt(m_player->lastError());
        return;
    }
    m_bufferTimer.start();
    m_playbackStatus =
        "Буферизация · " + candidate.value("source").toString() + " · " + candidate.value("title").toString();
    emit playbackChanged();
}

void HomeFeed::resolveStream(const QVariantMap& requested)
{
    const auto candidate = requested;
    cancelResolution();
    m_candidate = candidate;
    if (candidate.isEmpty()) {
        m_playWhenReady = false;
        m_playVkWhenReady = false;
        m_playbackStatus.clear();
        emit playbackChanged();
        return;
    }
    m_totalResolveTimer.start();
    m_resolvingId = candidate.value("libraryTrackId").toInt();
    m_resolvingUrl = candidate.value("url").toString();
    m_playbackStatus = "Подключаем «" + candidate.value("title").toString() + "»…";
    emit playbackChanged();
    const auto originalUrl = candidate.value("url").toString();
    const auto cached = m_streamCache.value(candidate.value("key").toString());
    if (cached.first > QDateTime::currentMSecsSinceEpoch() - 120000) {
        m_attemptedUrls.insert(originalUrl);
        finishStream(cached.second, {});
        return;
    }
    const QUrl direct(candidate.value("mediaUrl").toString());
    if (direct.scheme() == "https" && direct.userInfo().isEmpty() &&
        (MediaSniffer::isAudioUrl(direct) || candidate.value("mediaVerified").toBool())) {
        m_attemptedUrls.insert(originalUrl);
        finishStream(candidate, {});
        return;
    }
    const auto service = WebLoginManager::serviceForHost(QUrl(originalUrl).host());
    m_sourceGraceTimer.start();
    if (!service.isEmpty() && m_web->hasSession(service)) {
        static int serial = 0;
        m_attemptedUrls.insert(originalUrl);
        m_mediaRequest = -2000000 - (++serial);
        m_resolveTimer.start();
        m_web->resolveMedia(m_mediaRequest, candidate);
    } else if (service.isEmpty() && m_preview && m_preview->sources().contains(candidate.value("source").toString()) &&
               QUrl(originalUrl).scheme() == "https") {
        m_attemptedUrls.insert(originalUrl);
        m_previewPending = true;
        m_resolveTimer.start();
        m_preview->previewCandidate(candidate);
    } else {
        m_fallbackAllowed = true;
    }
    startLookup();
    tryAlternative();
}
