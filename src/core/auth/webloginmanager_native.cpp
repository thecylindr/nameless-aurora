#include "../web/mediasniffer.h"
#include "../web/vktext.h"
#include "../web/servicepayload.h"
#include "browsersession.h"
#include "webloginmanager.h"
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalServer>
#include <QSet>
#include <QSaveFile>
#include <QRegularExpression>
#include <QUrlQuery>
#include <QStandardPaths>
#include <QTimer>
#ifdef AURORA_WEBENGINE
#include <QWebEngineCookieStore>
#endif

// Нативная часть WebLoginManager: запуск браузера/профиля, инжекция скриптов,
// сбор плейлистов, поиск VK, получение аудио и сохранение отчётов об импорте.

QString WebLoginManager::serviceForHost(const QString& input)
{
    // Сопоставляем хост с известными доменами VK и Яндекса.
    const QString host = input.toLower();
    for (const auto& domain : QStringList{"vk.com", "vk.ru", "vk.me", "vk.id"})
        if (host == domain || host.endsWith('.' + domain))
            return "vk";
    for (const auto& domain : QStringList{"yandex.ru", "yandex.com", "yandex.by", "yandex.kz", "yandex.uz"})
        if (host == domain || host.endsWith('.' + domain))
            return "yandex";
    return {};
}
bool WebLoginManager::ensureNative(const QString& slug)
{
    if (m_selectedBrowser.isEmpty() || !QFileInfo(m_selectedBrowser).isExecutable()) {
        const auto browsers = detectBrowserExecutables();
        if (!browsers.isEmpty())
            m_selectedBrowser = browsers.first();
    }
    m_externalMode = !m_selectedBrowser.isEmpty();
    if (m_backgroundMode && !m_externalMode) {
        setStatus("Для фонового импорта нужен установленный Brave, Chromium, Chrome или Яндекс Браузер.");
        return false;
    }
    if (m_externalMode) {
        m_nativeProfile = nullptr;
        m_nativeSlug = slug;
        if (!m_nativeTimer) {
            m_nativeTimer = new QTimer(this);
            m_nativeTimer->setInterval(900);
            connect(m_nativeTimer, &QTimer::timeout, this, &WebLoginManager::nativeTick);
        }
        if (!m_external) {
            m_external = m_browserFactory ? m_browserFactory(this) : new BrowserSession(this);
            connect(m_external, &BrowserSession::loaded, this, &WebLoginManager::pageLoaded);
            connect(m_external, &BrowserSession::evaluated, this, &WebLoginManager::submitScriptResult);
            connect(m_external, &BrowserSession::apiReceived, this, &WebLoginManager::consumeApiPayload);
            connect(m_external, &BrowserSession::apiFetched, this,
                    [this](int id, const QUrl& url, const QJsonValue& payload, int status, int retryAfterMs) {
                        if (m_nativeTask == Wave && id == m_apiNextId && BrowserSession::isWaveApi(url)) {
                            const auto tracks = ServicePayload::yandexWave(payload);
                            if (status < 200 || status >= 300) {
                                finishNative(status == 401 || status == 403
                                                 ? "Яндекс отклонил сессию. Войди в аккаунт заново."
                                                 : "Не удалось получить волну Яндекса. Повтори позже.");
                            } else {
                                finishNative();
                                emit waveReady(tracks, {});
                            }
                            return;
                        }
                        receiveYandex(id, url, payload, status, retryAfterMs);
                    });
            connect(m_external, &BrowserSession::failed, this, [this](const QString& error) {
                if (m_nativeTask != Idle)
                    finishNative(error);
            });
            connect(m_external, &BrowserSession::cookiesReceived, this,
                    [this](const QString& service, const QJsonArray& values) {
                        QList<StoredCookie> relevant;
                        for (const auto& cookie : decodeCookies(values))
                            if (serviceForHost(cookie.domain.startsWith('.') ? cookie.domain.mid(1) : cookie.domain) ==
                                service)
                                relevant.append(cookie);
                        saveCookies(service, relevant);
                        emit sessionsChanged();
                    });
            connect(m_external, &BrowserSession::audioRequested, this, [this](const QUrl& url) {
                if (m_nativeTask != Resolve || !m_mediaArmed)
                    return;
                const int id = m_mediaId;
                finishNative();
                emit mediaReady(id, url, {});
            });
        }
        return true;
    }
#ifdef AURORA_WEBENGINE
    if (!m_profiles.contains(slug)) {
        // Chromium требует локальный IPC; проверяем заранее, до фатального
        // запуска в песочнице в ограниченных сессиях.
        QTemporaryDir probeDirectory;
        QLocalServer probe;
        if (!probeDirectory.isValid() || !probe.listen(probeDirectory.filePath("browser-ipc"))) {
            setStatus("Встроенный браузер недоступен: среда запрещает локальный обмен данными. Открой Aurora "
                      "в обычной сессии рабочего стола.");
            return false;
        }
        probe.close();
        auto* profile = new QQuickWebEngineProfile("Aurora-" + slug, this);
        const QString root = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/webprofiles/" + slug;
        QDir().mkpath(root);
        QFile::setPermissions(root, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
        profile->setPersistentStoragePath(root);
        profile->setCachePath(root + "/cache");
        profile->setPersistentCookiesPolicy(QQuickWebEngineProfile::ForcePersistentCookies);
        profile->setHttpAcceptLanguage("ru-RU,ru;q=0.9,en;q=0.7");
        // Перехватываем сетевые запросы профиля, чтобы ловить аудио-ссылки.
        auto* sniffer = new MediaSniffer(slug, profile);
        profile->setUrlRequestInterceptor(sniffer);
        connect(
            sniffer, &MediaSniffer::audioRequested, this,
            [this, slug](const QUrl& url) {
                if (m_nativeTask != Resolve || slug != m_nativeSlug || !m_mediaArmed)
                    return;
                const int id = m_mediaId;
                m_nativeVisible = false;
                finishNative();
                emit nativeBrowserChanged();
                emit mediaReady(id, url, {});
            },
            Qt::QueuedConnection);
        auto* store = profile->cookieStore();
        // Держим cookies встроенного профиля в QSettings для использования yt-dlp.
        connect(store, &QWebEngineCookieStore::cookieAdded, this, [this, slug](const QNetworkCookie& cookie) {
            const QString domain = cookie.domain().startsWith('.') ? cookie.domain().mid(1) : cookie.domain();
            if (serviceForHost(domain) != slug)
                return;
            auto cookies = storedCookies(slug);
            StoredCookie value{QString::fromUtf8(cookie.name()),
                               QString::fromUtf8(cookie.value()),
                               cookie.domain(),
                               cookie.path(),
                               cookie.isSessionCookie() ? 0 : cookie.expirationDate().toSecsSinceEpoch(),
                               cookie.isSecure(),
                               cookie.isHttpOnly()};
            for (qsizetype i = cookies.size(); i > 0; --i)
                if (cookies[i - 1].name == value.name && cookies[i - 1].domain == value.domain &&
                    cookies[i - 1].path == value.path)
                    cookies.removeAt(i - 1);
            cookies.append(value);
            saveCookies(slug, cookies);
            emit sessionsChanged();
        });
        connect(store, &QWebEngineCookieStore::cookieRemoved, this, [this, slug](const QNetworkCookie& cookie) {
            auto cookies = storedCookies(slug);
            for (qsizetype i = cookies.size(); i > 0; --i)
                if (cookies[i - 1].name == QString::fromUtf8(cookie.name()) &&
                    cookies[i - 1].domain == cookie.domain() && cookies[i - 1].path == cookie.path())
                    cookies.removeAt(i - 1);
            saveCookies(slug, cookies);
            emit sessionsChanged();
        });
        for (const auto& cookie : cookiesFor(slug))
            store->setCookie(cookie);
        store->loadAllCookies();
        m_profiles[slug] = profile;
    }
    m_nativeProfile = m_profiles.value(slug);
    m_nativeSlug = slug;
    for (const auto& cookie : cookiesFor(slug))
        m_nativeProfile->cookieStore()->setCookie(cookie);
    if (!m_nativeTimer) {
        m_nativeTimer = new QTimer(this);
        m_nativeTimer->setInterval(900);
        connect(m_nativeTimer, &QTimer::timeout, this, &WebLoginManager::nativeTick);
    }
    return true;
#else
    setStatus("Установи Chrome, Edge, Brave или Яндекс Браузер для входа в сервис.");
    return false;
#endif
}
void WebLoginManager::openNative(const QString& service)
{
    if (m_busy)
        return;
    const QString slug = normalizeService(service);
    if (slug.isEmpty())
        return;
    m_backgroundMode = false;
    // Сбрасываем привязку к локальному профилю: вход выполняет сам пользователь.
    m_store.remove("weblogin-local/" + slug);
    if (!ensureNative(slug)) {
        emit loginDone(service, false, m_status);
        return;
    }
    m_nativeTask = Login;
    m_nativeElapsed.start();
    m_pageError.clear();
    m_loadedUrl.clear();
    m_nativeUrl = QUrl(slug == "vk" ? "https://vk.ru/audio" : "https://music.yandex.ru/");
    m_nativeVisible = true;
    setBusy(true);
    setStatus(m_externalMode ? "Войди в аккаунт в открывшемся окне. Aurora определит вход автоматически."
                             : "Войди в аккаунт в окне Aurora, затем нажми «Я вошёл».");
    emit nativeBrowserChanged();
    m_nativeTimer->start();
    navigateNative(m_nativeUrl);
}
void WebLoginManager::setBrowserVisible(bool visible)
{
    m_nativeVisible = visible;
    emit nativeBrowserChanged();
    if (!visible && m_nativeTask != Idle)
        finishNative("Окно сервиса закрыто. Операция отменена.");
}
void WebLoginManager::pageLoaded(const QUrl& url, bool ok, const QString& error)
{
    m_loadedUrl = url;
    if (!ok) {
        m_pageError = error.isEmpty()
                          ? QStringLiteral("Не удалось открыть сервис. Проверь соединение и попробуй ещё раз.")
                          : error;
        emit nativeBrowserChanged();
        if (m_nativeTask != Idle)
            finishNative(m_pageError);
        return;
    }
    m_pageError.clear();
    emit nativeBrowserChanged();
}
void WebLoginManager::confirmNativeLogin()
{
    if (!m_externalMode) {
#ifdef AURORA_WEBENGINE
        if (m_nativeProfile)
            m_nativeProfile->cookieStore()->loadAllCookies();
        else
#endif
            return;
    }
    QTimer::singleShot(250, this, [this] {
        // Если во время сбора сервис попросил вход — возвращаемся к исходной странице.
        if (m_nativeTask == Collect && m_collectLogin) {
            m_collectLogin = false;
            m_loadedUrl.clear();
            emit nativeBrowserChanged();
            navigateNative(m_nativeUrl);
            return;
        }
        if (m_nativeTask != Login)
            return;
        if (!hasSession(m_nativeSlug)) {
            setStatus("Вход ещё не завершён. Заверши его на сайте и нажми «Я вошёл».");
            return;
        }
        const QString service = nativeService();
        finishNative();
        m_nativeVisible = false;
        emit nativeBrowserChanged();
        emit sessionsChanged();
        emit loginDone(service, true, service + " подключён.");
    });
}
QString WebLoginManager::nativeScript(const QString& name) const
{
    QFile file(":/native/scripts/" + name);
    return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.readAll()) : QString();
}
void WebLoginManager::collectPlaylist(const QUrl& url)
{
    m_backgroundMode = false;
    startCollection(url);
}
void WebLoginManager::startCollection(const QUrl& url, bool search, bool recommendations)
{
    if (m_busy) {
        if (recommendations)
            emit vkRecommendationsReady({}, "Дождись завершения операции VK.");
        else if (search)
            emit searchReady({}, "Дождись завершения операции VK.");
        else
            emit collectionReady({}, {}, "Сначала заверши текущую операцию входа или импорта.");
        return;
    }
    const QString slug = serviceForHost(url.host());
    if (slug.isEmpty() || (url.scheme() != "https" && url.scheme() != "http")) {
        if (recommendations)
            emit vkRecommendationsReady({}, "Неизвестный источник рекомендаций.");
        else if (search)
            emit searchReady({}, "Неизвестный источник поиска.");
        else
            emit collectionReady({}, {}, "Нужна ссылка Яндекс Музыки или VK Музыки.");
        return;
    }
    if (!ensureNative(slug)) {
        if (recommendations)
            emit vkRecommendationsReady({}, m_status);
        else if (search)
            emit searchReady({}, m_status);
        else
            emit collectionReady({}, {}, m_status);
        return;
    }
    m_nativeTask = recommendations ? Recommend : search ? Search : Collect;
    m_collectLogin = false;
    m_nativeElapsed.start();
    m_harvest.clear();
    m_harvestOrder.clear();
    m_harvestDiagnostic = {};
    m_harvestPage = QUrl();
    m_expectedTracks = 0;
    m_importReport.clear();
    m_vkEntryRedirected = false;
    ++m_apiGeneration;
    m_apiNativeScript = 0;
    m_apiRetryPending.clear();
    m_apiPages.clear();
    m_apiTrackAttempts.clear();
    m_apiBase = "https://api.music.yandex.ru";
    m_apiOwner.clear();
    m_apiKind.clear();
    m_apiTotalKnown = false;
    m_apiManifestComplete = false;
    m_apiLegacyRequested = false;
    m_apiProgress.start();
    m_apiPlaylist.clear();
    m_apiOrder.clear();
    m_apiRequests.clear();
    m_apiQueue.clear();
    m_apiRequested.clear();
    m_apiResponses = 0;
    m_apiHttpStatus = 0;
    m_stableTicks = 0;
    m_scriptPending = false;
    m_pageError.clear();
    m_collectionName.clear();
    m_nativeUrl = url;
    // Ссылку «/collection/tracks» приводим к общей коллекции, а плейлист запоминаем по id.
    if (slug == "yandex" && m_nativeUrl.path() == "/collection/tracks")
        m_nativeUrl.setPath("/collection");
    if (slug == "yandex") {
        const auto match = QRegularExpression("^/playlists/([A-Za-z0-9_.-]{1,100})/?$").match(m_nativeUrl.path());
        if (match.hasMatch())
            m_apiPlaylist = match.captured(1);
    }
    m_loadedUrl.clear();
    m_nativeVisible = true;
    setBusy(true);
    setStatus(m_backgroundMode ? "Читаем музыку с существующей сессией браузера…" : "Открываем плейлист в Aurora…");
    emit nativeBrowserChanged();
    m_nativeTimer->start();
    navigateNative(m_nativeUrl);
}
void WebLoginManager::requestYandex(const QUrl& url, const QJsonArray& ids)
{
    if (m_nativeTask != Collect || !BrowserSession::isCollectionApi(url))
        return;
    const auto key = url.toString();
    if (ids.isEmpty() && m_apiRequested.contains(key))
        return;
    if (ids.isEmpty())
        m_apiRequested.insert(key);
    m_apiQueue.enqueue({url, ids, 0});
    dispatchYandex();
}
void WebLoginManager::dispatchYandex()
{
    if (m_nativeTask != Collect || !m_apiRequests.isEmpty() || !m_apiRetryPending.isEmpty() || m_apiQueue.isEmpty())
        return;
    const auto request = m_apiQueue.dequeue();
    const int id = ++m_apiNextId;
    m_apiRequests[id] = request;
    if (m_externalMode && m_external) {
        m_external->fetchJson(id, request.url, request.ids);
    } else {
        m_scriptPending = true;
        m_apiNativeRequest = id;
        m_apiNativeScript = ++m_scriptId;
        const int script = m_apiNativeScript;
        emit scriptRequested(script, BrowserSession::jsonFetchScript(request.url, {}, request.ids));
        QTimer::singleShot(14000, this, [this, id, script, request] {
            if (m_apiNativeScript != script || !m_apiRequests.contains(id))
                return;
            m_apiNativeScript = 0;
            m_scriptPending = false;
            receiveYandex(id, request.url, {}, 0);
        });
    }
}
void WebLoginManager::receiveYandex(int id, const QUrl& url, const QJsonValue& payload, int status, int retryAfterMs)
{
    if (m_nativeTask != Collect || !m_apiRequests.contains(id))
        return; // A response from a cancelled or previous import.
    auto request = m_apiRequests.take(id);
    if (request.url != url) {
        failYandexIncomplete("Получен ответ другого запроса API.");
        return;
    }
    m_apiHttpStatus = status;
    if (status >= 200 && status < 300 && (payload.isObject() || payload.isArray())) {
        consumeApiPayload(url, payload);
    } else if ((status == 0 || status == 408 || status == 429 || status >= 500 ||
                ((status == 401 || status == 403) && request.attempt == 0 && m_apiResponses == 0)) && request.attempt < 2) {
        ++request.attempt;
        if (status == 0 && request.attempt == 1)
            request.url.setHost(url.host() == "api.music.yandex.ru" ? "api.music.yandex.net" : "api.music.yandex.ru");
        const auto key = QString::number(id);
        m_apiRetryPending.insert(key);
        const int generation = m_apiGeneration;
        setStatus(QString("Яндекс временно не ответил · повтор %1/2 · получено %2 / %3")
                      .arg(request.attempt).arg(m_harvest.size()).arg(m_expectedTracks));
        const int delay = qMax((status == 429 ? 1500 : 300) * request.attempt, qBound(0, retryAfterMs, 60000));
        QTimer::singleShot(delay, this,
                          [this, key, generation, request] {
            if (m_nativeTask != Collect || generation != m_apiGeneration)
                return;
            m_apiRetryPending.remove(key);
            m_apiQueue.prepend(request);
            dispatchYandex();
        });
    } else if (url.path().startsWith("/users/") && status != 401 && status != 403) {
        // The modern endpoint can still paginate if this account has no legacy route.
        advanceYandex();
    } else {
        failYandexIncomplete(status == 401 || status == 403
                                 ? QString("Яндекс не принял сессию (HTTP %1). Войди в аккаунт заново.").arg(status)
                                 : QString("Запрос к API не завершён (HTTP %1).").arg(status));
    }
    if (m_nativeTask == Collect) {
        dispatchYandex();
        finishApiCollection();
    }
}
void WebLoginManager::consumeApiPayload(const QUrl& url, const QJsonValue& payload)
{
    if (m_nativeTask == Wave && BrowserSession::isWaveApi(url)) {
        const auto tracks = ServicePayload::yandexWave(payload);
        if (!tracks.isEmpty()) {
            finishNative();
            emit waveReady(tracks, {});
        }
        return;
    }
    if (m_nativeTask != Collect || m_nativeSlug != "yandex" || !BrowserSession::isCollectionApi(url))
        return;
    const bool preview = url.path() == "/landing-blocks/collection/playlist-with-likes";
    const bool playlist = url.path().startsWith("/playlist/");
    const bool legacy = url.path().startsWith("/users/");
    const bool collection = m_nativeUrl.path() == "/collection" || m_nativeUrl.path() == "/collection/";
    const auto data = ServicePayload::yandex(payload);
    if (preview) {
        if (!collection || data.playlistUuid.isEmpty())
            return;
        m_apiPlaylist = data.playlistUuid;
    } else if (playlist) {
        if (m_apiPlaylist.isEmpty() || url.path() != "/playlist/" + m_apiPlaylist)
            return;
        if (!data.playlistUuid.isEmpty() && data.playlistUuid != m_apiPlaylist) {
            failYandexIncomplete("API вернул другой плейлист.");
            return;
        }
    } else if (legacy) {
        if (m_apiOwner.isEmpty() || url.path() != "/users/" + m_apiOwner + "/playlists/" + m_apiKind)
            return;
        if ((!data.ownerId.isEmpty() && data.ownerId != m_apiOwner) ||
            (!data.kind.isEmpty() && data.kind != m_apiKind)) {
            failYandexIncomplete("API вернул другой плейлист или владельца.");
            return;
        }
    } else if (m_apiPlaylist.isEmpty()) {
        return;
    }
    m_apiBase = "https://" + url.host();
    ++m_apiResponses;
    const int before = m_harvest.size(), beforeOrder = m_apiOrder.size(), beforeTotal = m_expectedTracks;
    if (preview || playlist || legacy) {
        m_expectedTracks = qMax(m_expectedTracks, data.total); // Never shrink to a later page's size.
        m_apiTotalKnown = m_apiTotalKnown || data.totalKnown;
        if (m_expectedTracks > 10000 || data.order.size() > 10000) {
            finishNative("За один импорт можно получить до 10 000 треков.");
            return;
        }
        if (!data.title.isEmpty())
            m_collectionName = data.title;
        if (!data.ownerId.isEmpty() && !data.kind.isEmpty()) {
            m_apiOwner = data.ownerId;
            m_apiKind = data.kind;
        }
    }
    for (const auto& track : data.tracks) {
        // Metadata endpoints may return extra entries. Membership comes from the manifest only.
        if (url.path() == "/tracks" && !m_apiOrder.contains(track.sourceUrl))
            continue;
        m_harvest[track.sourceUrl] = track;
    }
    if ((playlist || legacy) && (data.fullPlaylist || m_apiTotalKnown)) {
        if (m_expectedTracks == 0 && data.order.isEmpty()) {
            finishNative("Сервис сообщает, что этот список музыки пуст.");
            return;
        }
        bool validPage = false;
        const auto pageText = QUrlQuery(url).queryItemValue("page");
        const int page = pageText.isEmpty() ? 0 : pageText.toInt(&validPage);
        if ((!pageText.isEmpty() && !validPage) || page < 0 || page > 50)
            return;
        // A later page can arrive first through CDP and report only its page size.
        // It cannot establish a complete manifest without the beginning of the list.
        const bool complete = (legacy || data.manifest || page == 0) && data.order.size() >= m_expectedTracks;
        if (complete) {
            m_apiPages.clear();
            m_apiPages[0] = data.order;
            m_apiManifestComplete = true;
        } else if (!m_apiManifestComplete) {
            bool repeated = false;
            for (auto it = m_apiPages.cbegin(); it != m_apiPages.cend(); ++it)
                repeated = repeated || (it.key() != page && it.value() == data.order);
            if (data.order.isEmpty() || repeated) {
                if (!m_apiLegacyRequested && !m_apiOwner.isEmpty()) {
                    m_apiLegacyRequested = true;
                    requestYandex(QUrl(m_apiBase + "/users/" + m_apiOwner + "/playlists/" + m_apiKind));
                    return;
                }
                failYandexIncomplete(repeated ? "API повторил ту же страницу вместо продолжения."
                                              : "API вернул пустую страницу до конца плейлиста.");
                return;
            }
            // A small passive preview must not replace a more complete page already received.
            if (data.order.size() >= m_apiPages.value(page).size())
                m_apiPages[page] = data.order;
        }
        m_apiOrder.clear();
        QSet<QString> seen;
        for (int page = 0; m_apiPages.contains(page); ++page)
            for (const auto& key : m_apiPages.value(page))
                if (!seen.contains(key)) {
                    seen.insert(key);
                    m_apiOrder.append(key);
                }
        m_apiManifestComplete = m_apiTotalKnown && m_apiOrder.size() >= m_expectedTracks;
    }
    if (before != m_harvest.size() || beforeOrder != m_apiOrder.size() || beforeTotal != m_expectedTracks)
        m_apiProgress.restart();
    if (m_apiManifestComplete) {
        int ready = 0;
        for (const auto& key : m_apiOrder)
            ready += m_harvest.contains(key);
        setStatus(QString("Получено треков Яндекса: %1 / %2 · %3%")
                      .arg(ready).arg(m_apiOrder.size()).arg(m_apiOrder.isEmpty() ? 0 : 100 * ready / m_apiOrder.size()));
    }
    advanceYandex();
}
void WebLoginManager::advanceYandex()
{
    if (m_nativeTask != Collect || !m_apiRequests.isEmpty() || !m_apiQueue.isEmpty() || !m_apiRetryPending.isEmpty())
        return;
    if (!m_apiManifestComplete) {
        if (!m_apiLegacyRequested && !m_apiOwner.isEmpty() && !m_apiKind.isEmpty()) {
            m_apiLegacyRequested = true;
            requestYandex(QUrl(m_apiBase + "/users/" + m_apiOwner + "/playlists/" + m_apiKind));
            return;
        }
        int page = 0;
        while (m_apiPages.contains(page))
            ++page;
        if (page > 50) {
            failYandexIncomplete("Достигнут предел страниц API.");
            return;
        }
        if (!m_apiPlaylist.isEmpty())
            requestYandex(QUrl(m_apiBase + "/playlist/" + m_apiPlaylist +
                               "?resumeStream=false&richTracks=true&pageSize=200&page=" + QString::number(page)));
        setStatus(QString("Читаем список Яндекса: %1 / %2 · страница %3")
                      .arg(m_apiOrder.size()).arg(m_expectedTracks).arg(page + 1));
        return;
    }
    QJsonArray batch;
    const auto enqueueBatch = [&] {
        if (!batch.isEmpty()) {
            requestYandex(QUrl(m_apiBase + "/tracks"), batch);
            batch = {};
        }
    };
    int ready = 0;
    for (const auto& key : m_apiOrder) {
        if (m_harvest.contains(key)) {
            ++ready;
            continue;
        }
        if (++m_apiTrackAttempts[key] > 3) {
            failYandexIncomplete("API не вернул метаданные части записей после трёх попыток.");
            return;
        }
        batch.append(QUrl(key).path().section('/', -1));
        if (batch.size() == 200)
            enqueueBatch();
    }
    enqueueBatch();
    setStatus(QString("Получено треков Яндекса: %1 / %2 · %3%")
                  .arg(ready).arg(m_apiOrder.size()).arg(m_apiOrder.isEmpty() ? 0 : 100 * ready / m_apiOrder.size()));
    finishApiCollection();
}
void WebLoginManager::failYandexIncomplete(const QString& reason)
{
    int ready = 0;
    for (const auto& key : m_apiOrder)
        ready += m_harvest.contains(key);
    const int expected = qMax(m_expectedTracks, int(m_apiOrder.size()));
    const auto progress = expected > 0 ? QString("Получено %1 из %2 треков. ").arg(ready).arg(expected)
                                      : QString("Полный размер плейлиста не подтверждён. ");
    finishNative("Яндекс: " + progress + reason + " Неполный список не сохранён; повтори импорт.");
}
bool WebLoginManager::finishApiCollection()
{
    if (m_nativeTask != Collect || !m_apiTotalKnown || !m_apiManifestComplete || m_apiOrder.isEmpty() ||
        !m_apiRequests.isEmpty() || !m_apiQueue.isEmpty() || !m_apiRetryPending.isEmpty() ||
        m_apiOrder.size() < m_expectedTracks)
        return false;
    QVector<Track> tracks;
    for (const auto& key : m_apiOrder) {
        if (!m_harvest.contains(key))
            return false;
        tracks.append(m_harvest.value(key));
    }
    const auto name = m_collectionName;
    finishNative();
    m_nativeVisible = false;
    emit nativeBrowserChanged();
    emit collectionReady(tracks, name, {});
    return true;
}
void WebLoginManager::resolveMedia(int requestId, const QVariantMap& track)
{
    const QUrl direct(track.value("mediaUrl").toString());
    if (MediaSniffer::isAudioUrl(direct)) {
        QTimer::singleShot(0, this, [this, requestId, direct] {
            emit mediaReady(requestId, direct, {});
        });
        return;
    }
    if (requestId < 0)
        m_mediaQueue.prepend({requestId, track});
    else
        m_mediaQueue.enqueue({requestId, track});
    nextMedia();
}
void WebLoginManager::nextMedia()
{
    if (m_busy || m_mediaQueue.isEmpty())
        return;
    const auto task = m_mediaQueue.dequeue();
    m_mediaId = task.first;
    m_requestedTrack = task.second;
    const QUrl url(m_requestedTrack.value("url").toString());
    const QString slug = serviceForHost(url.host());
    if (slug.isEmpty()) {
        emit mediaReady(m_mediaId, {}, "Неизвестный музыкальный сервис.");
        QTimer::singleShot(0, this, &WebLoginManager::nextMedia);
        return;
    }
    m_backgroundMode = !m_store.value("weblogin-local/" + slug).toString().isEmpty();
    if (!ensureNative(slug)) {
        emit mediaReady(m_mediaId, {}, m_status);
        QTimer::singleShot(0, this, &WebLoginManager::nextMedia);
        return;
    }
    m_nativeTask = Resolve;
    m_mediaArmed = false;
    m_nativeElapsed.start();
    m_scriptPending = false;
    m_nativeVisible = true;
    m_nativeUrl = url;
    if (slug == "vk" && m_requestedTrack.value("rawAudio").toList().isEmpty()) {
        const auto query =
            (m_requestedTrack.value("artist").toString() + " " + m_requestedTrack.value("title").toString()).trimmed();
        if (!query.isEmpty()) {
            m_nativeUrl = QUrl("https://vk.ru/audio");
            QUrlQuery search;
            search.addQueryItem("section", "search");
            search.addQueryItem("q", query.left(250));
            m_nativeUrl.setQuery(search);
        }
    }
    m_loadedUrl.clear();
    m_pageError.clear();
    setBusy(true);
    setStatus("Получаем аудио: " + m_requestedTrack.value("title").toString());
    emit nativeBrowserChanged();
    m_nativeTimer->start();
    navigateNative(m_nativeUrl);
}
void WebLoginManager::cancelMedia(int requestId)
{
    for (qsizetype i = m_mediaQueue.size(); i > 0; --i)
        if (m_mediaQueue[i - 1].first == requestId)
            m_mediaQueue.removeAt(i - 1);
    if (m_nativeTask == Resolve && m_mediaId == requestId)
        finishNative("Загрузка отменена.");
}
void WebLoginManager::requestWave(const QStringList& queue)
{
    if (m_nativeTask != Idle || busy()) {
        emit waveReady({}, "Сначала заверши вход, импорт или поиск в сервисе.");
        return;
    }
    if (!hasSession("yandex")) {
        emit waveReady({}, "Войди в Яндекс Музыку, чтобы получить свою волну.");
        return;
    }
    m_backgroundMode = false;
    if (!ensureNative("yandex")) {
        emit waveReady({}, m_status);
        return;
    }
    m_backgroundMode = m_externalMode;
    m_nativeTask = Wave;
    m_waveQueue.clear();
    for (const auto& id : queue) {
        if (QRegularExpression("^[0-9]{1,20}$").match(id).hasMatch())
            m_waveQueue.append(id);
        if (m_waveQueue.size() >= 50)
            break;
    }
    m_nativeElapsed.start();
    m_apiResponses = 0;
    m_scriptPending = false;
    m_loadedUrl.clear();
    m_pageError.clear();
    m_nativeUrl = QUrl("https://music.yandex.ru/");
    m_nativeVisible = !m_externalMode;
    setBusy(true);
    setStatus("Получаем «Мою волну» Яндекс Музыки…");
    emit nativeBrowserChanged();
    m_nativeTimer->start();
    navigateNative(m_nativeUrl);
}

void WebLoginManager::cancelWave()
{
    if (m_nativeTask == Wave)
        finishNative("Получение волны отменено.");
}

void WebLoginManager::nativeTick()
{
    if (m_nativeTask == Idle)
        return;
    m_nativeTimer->setInterval(m_nativeTask == Collect ? 500 : 900);
    // Свой лимит ожидания для каждой операции.
    const int limit = m_nativeTask == Login                                                         ? 180000
                      : m_nativeTask == Resolve                                                     ? 25000
                      : m_nativeTask == Search || m_nativeTask == Wave || m_nativeTask == Recommend ? 35000
                      : m_nativeTask == Collect && m_nativeSlug == "yandex"                        ? 600000
                                                                                                    : 240000;
    if (m_nativeElapsed.elapsed() > limit) {
        if (m_nativeTask == Collect && m_nativeSlug == "yandex") {
            failYandexIncomplete("Истекло время ожидания API.");
            return;
        }
        finishNative(
            m_nativeTask == Collect ? "Сервис не завершил выдачу списка. Попробуй открыть сам плейлист вместо страницы "
                                      "всей музыки."
            : m_nativeTask == Search ? "Поиск VK не завершился за 35 секунд. Повтори запрос или выбери другой источник."
                                     : "Время ожидания истекло. Повтори операцию или заверши вход в окне сервиса.");
        return;
    }
    if (m_loadedUrl.isEmpty() || m_scriptPending)
        return;
    if (serviceForHost(m_loadedUrl.host()) != m_nativeSlug)
        return;
    if (m_nativeTask == Wave) {
        // Даём странице восстановить заголовки аккаунта из её обычных запросов.
        if (m_nativeElapsed.elapsed() < 5000 || m_apiResponses)
            return;
        ++m_apiResponses;
        QUrl endpoint("https://api.music.yandex.ru/rotor/station/user:onyourwave/tracks?settings2=true");
        if (!m_waveQueue.isEmpty()) {
            QUrlQuery query(endpoint);
            query.addQueryItem("queue", m_waveQueue.join(','));
            endpoint.setQuery(query);
        }
        if (m_externalMode && m_external) {
            m_external->fetchJson(++m_apiNextId, endpoint);
        } else {
            m_scriptPending = true;
            const QString script =
                QStringLiteral("(async()=>{const c=new AbortController();const t=setTimeout(()=>c.abort(),12000);"
                               "try{const r=await fetch('%1',{credentials:'include',signal:c.signal});"
                               "const text=await r.text();return JSON.stringify({status:r.status,"
                               "payload:text.length<=2097152?JSON.parse(text):{}});}"
                               "catch(e){return JSON.stringify({status:0});}finally{clearTimeout(t);}})()")
                    .arg(endpoint.toString());
            emit scriptRequested(++m_scriptId, script);
        }
        return;
    }
    if (m_nativeTask == Collect && m_nativeSlug == "yandex" && !m_collectLogin && m_apiRequested.isEmpty() &&
        m_nativeElapsed.elapsed() > 2500) {
        if (!m_apiPlaylist.isEmpty())
            requestYandex(QUrl("https://api.music.yandex.ru/playlist/" + m_apiPlaylist +
                               "?resumeStream=false&richTracks=true&pageSize=200&page=0"));
        else if (m_nativeUrl.path() == "/collection" || m_nativeUrl.path() == "/collection/")
            requestYandex(QUrl("https://api.music.yandex.ru/landing-blocks/collection/playlist-with-likes?count=8"));
    }
    if (m_apiNativeScript != 0)
        return;
    QString script =
        nativeScript(m_nativeTask == Login                                                            ? "login.js"
                     : m_nativeTask == Collect || m_nativeTask == Search || m_nativeTask == Recommend ? "harvest.js"
                                                                                                      : "activate.js");
    if (m_nativeTask == Collect && m_nativeSlug == "vk")
        script = "window.__auroraCatalogSearch=false;\n" + nativeScript("vk-library.js") + ";\n" + script;
    if (m_nativeTask == Search || m_nativeTask == Recommend)
        script = "window.__auroraCatalogSearch=true;window.__auroraVkLibrary=null;window.__auroraRecommendations=" +
                 QString(m_nativeTask == Recommend ? "true" : "false") + ";\n" + script;
    if (m_nativeTask == Resolve)
        script.replace(
            "__TRACK__",
            QString::fromUtf8(
                QJsonDocument(QJsonObject::fromVariantMap(m_requestedTrack)).toJson(QJsonDocument::Compact)));
    if (script.isEmpty()) {
        finishNative("Не удалось загрузить модуль импорта.");
        return;
    }
    m_scriptPending = true;
    ++m_scriptId;
    if (m_externalMode && m_external)
        m_external->evaluate(m_scriptId, script, m_nativeTask == Resolve);
    else
        emit scriptRequested(m_scriptId, script);
    const int id = m_scriptId;
    QTimer::singleShot(7000, this, [this, id] {
        if (m_scriptPending && id == m_scriptId) {
            m_scriptPending = false;
            finishNative("Страница сервиса не отвечает. Обнови её и повтори импорт.");
        }
    });
}
QVector<Track> WebLoginManager::parseHarvest(const QByteArray& json, const QString& service)
{
    QVector<Track> tracks;
    QSet<QString> seen;
    const auto root = QJsonDocument::fromJson(json).object();
    for (const auto& entry : root.value("tracks").toArray()) {
        const auto object = entry.toObject();
        const QUrl url(object.value("url").toString());
        if (serviceForHost(url.host()) != normalizeService(service) || url.scheme() != "https")
            continue;
        Track t;
        t.title = VkText::clean(object.value("title").toString().trimmed());
        t.artist = VkText::clean(object.value("artist").toString().trimmed());
        t.source = normalizeService(service) == "vk" ? "VK" : "Яндекс Музыка";
        t.sourceUrl = url.toString();
        t.album = VkText::clean(object.value("album").toString());
        const QUrl cover(object.value("coverUrl").toString());
        if (cover.scheme() == "https")
            t.coverUrl = cover.toString();
        t.duration = qMax<qint64>(0, object.value("durationMs").toVariant().toLongLong());
        const QUrl media(object.value("mediaUrl").toString());
        if (MediaSniffer::isAudioUrl(media) ||
            (object.value("mediaVerified").toBool() && media.scheme() == "https" && media.userInfo().isEmpty()))
            t.mediaUrl = media.toString();
        t.webData = object.toVariantMap();
        if (t.title.isEmpty() || seen.contains(t.sourceUrl))
            continue;
        seen.insert(t.sourceUrl);
        tracks.append(t);
    }
    return tracks;
}
void WebLoginManager::submitScriptResult(int id, const QVariant& result)
{
    if (m_nativeTask == Collect && m_apiNativeScript != 0 && id == m_apiNativeScript) {
        const auto response = QJsonDocument::fromJson(result.toString().toUtf8()).object();
        const int request = m_apiNativeRequest;
        m_apiNativeScript = 0;
        m_scriptPending = false;
        if (m_apiRequests.contains(request))
            receiveYandex(request, m_apiRequests.value(request).url, response.value("payload"),
                          response.value("status").toInt(), qBound(0, response.value("retryAfterMs").toInt(), 60000));
        return;
    }
    if (id != m_scriptId || !m_scriptPending || m_nativeTask == Idle)
        return;
    m_scriptPending = false;
    if (!result.isValid())
        return; // Браузер ушёл с навигацией во время evaluate; nativeTick повторит в пределах таймаута.
    if (m_nativeTask == Wave) {
        const auto response = QJsonDocument::fromJson(result.toString().toUtf8()).object();
        const int status = response.value("status").toInt();
        if (status < 200 || status >= 300) {
            finishNative("Не удалось получить волну Яндекса. Проверь вход в аккаунт и повтори.");
        } else {
            const auto tracks = ServicePayload::yandexWave(response.value("payload"));
            finishNative();
            emit waveReady(tracks, {});
        }
        return;
    }
    if (m_nativeTask == Login) {
        const auto state = QJsonDocument::fromJson(result.toString().toUtf8()).object();
        if (state.value("authenticated").toBool() && hasSession(m_nativeSlug))
            confirmNativeLogin();
        return;
    }
    if (m_nativeTask == Resolve) {
        const auto state = QJsonDocument::fromJson(result.toString().toUtf8()).object();
        m_mediaArmed = m_mediaArmed || state.value("activated").toBool();
        const QUrl url(state.value("mediaUrl").toString());
        if (MediaSniffer::isAudioUrl(url)) {
            const int request = m_mediaId;
            m_nativeVisible = false;
            finishNative();
            emit nativeBrowserChanged();
            emit mediaReady(request, url, {});
        }
        return;
    }
    const QByteArray bytes = result.toString().toUtf8();
    const auto root = QJsonDocument::fromJson(bytes).object();
    if (m_nativeTask == Search || m_nativeTask == Recommend) {
        if (root.value("challenge").toBool() || root.value("loginRequired").toBool()) {
            finishNative("VK не принял сессию или показал проверку доступа. Выбери профиль браузера в импорте.");
            return;
        }
        const auto tracks = parseHarvest(bytes, "vk");
        for (const auto& track : tracks)
            if (!m_harvest.contains(track.sourceUrl)) {
                m_harvest.insert(track.sourceUrl, track);
                m_harvestOrder.append(track.sourceUrl);
            }
        if (!m_harvest.isEmpty() &&
            (m_nativeTask == Recommend ? (m_harvest.size() >= 200 || m_nativeElapsed.elapsed() > 28000 ||
                                          (m_nativeElapsed.elapsed() > 12000 && !root.value("loading").toBool()))
                                       : (m_harvest.size() >= 24 || m_nativeElapsed.elapsed() > 7000))) {
            QVector<Track> found;
            for (const auto& key : m_harvestOrder) {
                found.append(m_harvest.value(key));
                if (found.size() >= (m_nativeTask == Recommend ? 200 : 36))
                    break;
            }
            const bool recommendations = m_nativeTask == Recommend;
            finishNative();
            if (recommendations)
                emit vkRecommendationsReady(found, {});
            else
                emit searchReady(found, {});
            return;
        }
        if (m_nativeElapsed.elapsed() > 22000 && !root.value("loading").toBool())
            finishNative(
                m_nativeTask == Recommend
                    ? "Не удалось прочитать рекомендации VK. Открой раздел «Для вас» в аккаунте и обнови подборку."
                    : "VK не вернул музыкальных записей. Попробуй другой запрос или источник.");
        return;
    }
    if (root.isEmpty()) {
        finishNative("Не удалось прочитать страницу сервиса. Открой плейлист и повтори импорт.");
        return;
    }
    m_harvestDiagnostic = root.value("diagnostics").toObject();
    m_harvestPage = QUrl(root.value("url").toString());
    const QUrl entry(root.value("entryUrl").toString());
    if (m_nativeSlug == "vk" && !m_vkEntryRedirected &&
        QRegularExpression("^/audio/?$").match(m_nativeUrl.path()).hasMatch() && entry.scheme() == "https" &&
        entry.host() == m_nativeUrl.host() && entry.userInfo().isEmpty() &&
        (entry.port() == -1 || entry.port() == 443) &&
        QRegularExpression("^/audios[1-9][0-9]*$").match(entry.path()).hasMatch()) {
        m_vkEntryRedirected = true;
        m_nativeUrl = entry;
        m_loadedUrl.clear();
        m_stableTicks = 0;
        m_harvest.clear();
        m_harvestOrder.clear();
        setStatus("Открываем полную коллекцию VK…");
        navigateNative(m_nativeUrl);
        return;
    }
    m_expectedTracks = qMax(m_expectedTracks, qBound(0, root.value("expectedCount").toInt(), 100000));
    if (root.value("challenge").toBool()) {
        finishNative(nativeService() + " показал проверку безопасности вместо списка музыки. Список не прочитан; "
                                       "подробности доступны в отчёте импорта.");
        return;
    }
    if (root.value("loginRequired").toBool()) {
        if (m_backgroundMode) {
            finishNative("Сервис не принял сохранённую сессию. Выбери профиль браузера, где музыка сейчас открывается "
                         "под твоим аккаунтом, и повтори перенос.");
            return;
        }
        m_collectLogin = true;
        emit nativeBrowserChanged();
        setStatus(m_externalMode
                      ? "Войди в аккаунт на странице сервиса. Импорт продолжится автоматически."
                      : "Войди в аккаунт на странице сервиса и нажми «Я вошёл». Импорт продолжится автоматически.");
        return;
    }
    if (m_collectLogin) {
        m_collectLogin = false;
        m_loadedUrl.clear();
        emit nativeBrowserChanged();
        navigateNative(m_nativeUrl);
        return;
    }
    // DOM rows are a virtualized preview, never proof of a complete Yandex playlist.
    if (m_nativeSlug == "yandex") {
        if (!finishApiCollection() && m_apiRequests.isEmpty() && m_apiQueue.isEmpty() &&
            m_apiRetryPending.isEmpty() && m_apiProgress.elapsed() > 30000)
            failYandexIncomplete("Сервис не подтвердил полный список через API.");
        return;
    }
    const auto tracks = parseHarvest(bytes, m_nativeSlug);
    const int before = m_harvest.size();
    if (root.value("complete").toBool() && root.value("playlistOrdered").toBool() &&
        tracks.size() == m_expectedTracks) {
        // Виртуальный список может сначала показать середину плейлиста; после
        // загрузки полной модели заменяем порядок обнаружения на точный.
        m_harvest.clear();
        m_harvestOrder.clear();
    }
    for (const auto& t : tracks) {
        if (!m_harvest.contains(t.sourceUrl))
            m_harvestOrder.append(t.sourceUrl);
        m_harvest[t.sourceUrl] = t;
    }
    if (!root.value("title").toString().isEmpty())
        m_collectionName = root.value("title").toString();
    if (m_harvest.size() > 10000) {
        finishNative("За один импорт можно получить до 10 000 треков.");
        return;
    }
    if (m_harvest.size() == before && root.value("atBottom").toBool() && !root.value("loading").toBool())
        ++m_stableTicks;
    else
        m_stableTicks = 0;
    if (m_harvest.isEmpty()) {
        const int rows =
            m_harvestDiagnostic.value("modernRows").toInt() + m_harvestDiagnostic.value("legacyRows").toInt();
        setStatus(root.value("loading").toBool()
                      ? nativeService() + " загружает список музыки…"
                      : QString("Читаем список %1 · элементов на странице: %2…").arg(nativeService()).arg(rows));
        if (m_nativeElapsed.elapsed() > 30000) {
            finishNative("Страница сервиса открылась, но список музыки не удалось прочитать. Диагностика сохранена; "
                         "открой отчёт импорта.");
            return;
        }
    } else
        setStatus(
            m_expectedTracks > 0
                ? QString("Получено треков: %1 / %2 · загружаем список…").arg(m_harvest.size()).arg(m_expectedTracks)
                : QString("Получено треков: %1 · прокручиваем плейлист…").arg(m_harvest.size()));
    const auto apiCode = m_harvestDiagnostic.value("apiCode").toString();
    if (m_nativeSlug == "vk" && apiCode == "rate-limited" && !m_harvestDiagnostic.value("apiFailed").toBool()) {
        setStatus(QString("VK просит паузу · получено %1 / %2 · повтор через %3 с…")
                      .arg(m_harvest.size())
                      .arg(m_expectedTracks)
                      .arg(qMax(1, (m_harvestDiagnostic.value("apiWaitMs").toInt() + 999) / 1000)));
        return;
    }
    if (m_stableTicks < 4 || m_nativeElapsed.elapsed() < 8000)
        return;
    if (m_harvest.isEmpty() && m_nativeElapsed.elapsed() < 20000)
        return;
    if (m_nativeSlug == "vk" && !m_harvest.isEmpty() && m_expectedTracks <= m_harvest.size() &&
        !root.value("complete").toBool()) {
        if (m_nativeElapsed.elapsed() < 20000)
            return;
        finishNative(QString("Прочитано %1 треков, но VK не подтвердил полный размер списка. Это может быть только "
                             "первая часть коллекции. Полный импорт не завершён; открой отчёт импорта.")
                         .arg(m_harvest.size()));
        return;
    }
    QVector<Track> collection;
    for (const auto& key : m_harvestOrder)
        collection.append(m_harvest.value(key));
    const QString name = m_collectionName;
    if (collection.isEmpty()) {
        finishNative(root.value("knownEmpty").toBool() ? "Сервис сообщает, что этот список музыки пуст."
                                                       : "Страница открылась, но Aurora не распознала её список "
                                                         "музыки. Сохранён отчёт импорта с данными о странице.");
        return;
    }
    if (m_expectedTracks > collection.size()) {
        finishNative(
            QString("Сервис показал %1 из %2 треков. Неполный список не сохранён; подробности — в отчёте импорта.")
                .arg(collection.size())
                .arg(m_expectedTracks));
        return;
    }
    finishNative();
    m_nativeVisible = false;
    emit nativeBrowserChanged();
    emit collectionReady(
        collection, name,
        collection.isEmpty()
            ? "На этой странице не найдено треков. Открой музыкальный плейлист или раздел «Моя музыка»."
            : QString());
}
void WebLoginManager::finishNative(const QString& error)
{
    // Сохраняем тип задачи и id медиа до сброса, чтобы корректно отдать результат.
    const auto task = m_nativeTask;
    const int media = m_mediaId;
    const QString service = nativeService();
    if (task == Collect && !error.isEmpty())
        saveImportReport(error);
    m_nativeTask = Idle;
    ++m_apiGeneration;
    m_apiNativeScript = 0;
    m_apiRetryPending.clear();
    if (task == Wave || task == Recommend)
        m_nativeVisible = false;
    m_apiRequests.clear();
    m_apiQueue.clear();
    m_collectLogin = false;
    m_scriptPending = false;
    ++m_scriptId;
    if (m_nativeTimer)
        m_nativeTimer->stop();
    setBusy(false);
    if (m_externalMode && m_external)
        m_external->hide();
    emit nativeBrowserChanged();
    if (!error.isEmpty()) {
        setStatus(error);
        if (task == Login)
            emit loginDone(service, false, error);
        else if (task == Search || task == ReadSearchSession)
            emit searchReady({}, error);
        else if (task == Collect || task == ReadLocalSession)
            emit collectionReady({}, {}, error);
        else if (task == Resolve)
            emit mediaReady(media, {}, error);
        else if (task == Wave)
            emit waveReady({}, error);
        else if (task == Recommend)
            emit vkRecommendationsReady({}, error);
    }
    QTimer::singleShot(0, this, &WebLoginManager::nextMedia);
}
void WebLoginManager::saveImportReport(const QString& error)
{
    // Собираем только числовые/логические диагностики и известные коды API.
    QJsonObject stats;
    for (const auto& key :
         QStringList{"version",     "legacyRows",         "modernRows",      "fullIdRows",   "stateLists",
                     "stateTracks", "recognized",         "expectedCount",   "frames",       "modernModels",
                     "fiberNodes",  "playlistCandidates", "modelReadErrors", "modelDepth",   "apiPages",
                     "apiTracks",   "apiRetries",         "apiWaitMs",       "scrollHeight", "viewportHeight"})
        stats.insert(key, qBound(0, m_harvestDiagnostic.value(key).toInt(), 100000));
    for (const auto& key :
         QStringList{"loginRequired", "challenge", "loading", "knownEmpty", "apiPending", "apiFailed"})
        stats.insert(key, m_harvestDiagnostic.value(key).toBool());
    const auto apiCode = m_harvestDiagnostic.value("apiCode").toString();
    if (QStringList{"idle", "loading", "complete", "unsupported", "denied", "incomplete", "stalled", "rate-limited",
                    "timeout", "network", "wrong-owner", "wrong-playlist", "size-limit", "response-size"}
            .contains(apiCode))
        stats["apiCode"] = apiCode;
    const auto ready = m_harvestDiagnostic.value("readyState").toString();
    if (QStringList{"loading", "interactive", "complete"}.contains(ready))
        stats.insert("readyState", ready);
    const QUrl page = serviceForHost(m_harvestPage.host()) == m_nativeSlug ? m_harvestPage : m_nativeUrl;
    const QString folder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/diagnostics";
    if (!QDir().mkpath(folder))
        return;
    QFile::setPermissions(folder, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    const auto path = folder + "/import-" + m_nativeSlug + ".json";
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return;
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    const auto bytes =
        QJsonDocument(
            QJsonObject{
                {"service", m_nativeSlug},
                {"utc", QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
                {"page", page.adjusted(QUrl::RemoveQuery | QUrl::RemoveFragment | QUrl::RemoveUserInfo).toString()},
                {"background", m_backgroundMode},
                {"collected", m_harvest.size()},
                {"error", error},
                {"pageStructure", stats},
                {"api", QJsonObject{{"responses", m_apiResponses},
                                    {"expected", m_expectedTracks},
                                    {"ordered", m_apiOrder.size()},
                                    {"pending", m_apiRequests.size()},
                                    {"queued", m_apiQueue.size()},
                                    {"httpStatus", m_apiHttpStatus},
                                    {"pages", m_apiPages.size()},
                                    {"totalKnown", m_apiTotalKnown},
                                    {"manifestComplete", m_apiManifestComplete},
                                    {"retryPending", m_apiRetryPending.size()}}},
                {"browserProtocol", m_external ? m_external->protocolDiagnostic() : QJsonObject{}}})
            .toJson(QJsonDocument::Indented);
    if (file.write(bytes) == bytes.size() && file.commit())
        m_importReport = path;
}
void WebLoginManager::openImportReport()
{
    if (!m_importReport.isEmpty() && QFileInfo::exists(m_importReport))
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_importReport));
}
