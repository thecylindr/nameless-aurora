#include "webloginmanager.h"
#include <QUrlQuery>
#include "../settingsstore.h"
#include "browsersession.h"
#include "browsercookieimporter.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTimer>
#ifdef AURORA_WEBENGINE
#include <QWebEngineCookieStore>
#endif

// Реализация WebLoginManager: cookies сессий, формат Netscape, запуск входа
// и импорт через внешний браузер или встроенный профиль.

namespace
{
const int pollIntervalMs = 2000;
const int loginTimeoutMs = 3 * 60 * 1000;
// Домены VK, на которых действует сессия.
QStringList vkDomains()
{
    return {QStringLiteral("vk.com"), QStringLiteral("vk.ru")};
}
// Проверяет принадлежность хоста домену (с учётом поддоменов и ведущей точки).
bool domainMatches(const QString& host, const QString& domain)
{
    const QString clean = domain.startsWith(QLatin1Char('.')) ? domain.mid(1) : domain;
    return host == clean || host.endsWith(QLatin1Char('.') + clean);
}
// Относится ли cookie к нужному сервису.
bool relevantFor(const QString& slug, const QString& domain)
{
    if (slug == QStringLiteral("vk"))
        return domainMatches(domain, QStringLiteral("vk.com")) || domainMatches(domain, QStringLiteral("vk.ru"));
    return WebLoginManager::serviceForHost(domain.startsWith('.') ? domain.mid(1) : domain) == QStringLiteral("yandex");
}
} // namespace

WebLoginManager::WebLoginManager(SettingsStore* settings, QObject* parent,
                                 std::function<BrowserSession*(QObject*)> browserFactory,
                                 BrowserCookieImporter* cookieImporter)
    : QObject(parent), m_settings(settings), m_browserFactory(std::move(browserFactory))
{
    m_cookieImporter = cookieImporter ? cookieImporter : new BrowserCookieImporter(settings, this);
    connect(m_cookieImporter, &BrowserCookieImporter::statusChanged, this, &WebLoginManager::setStatus);
    connect(m_cookieImporter, &BrowserCookieImporter::finished, this,
            [this](const QString& slug, const QString& profileId, const QJsonArray& values, const QString& error) {
                if (m_nativeTask != ReadLocalSession && m_nativeTask != ReadSearchSession)
                    return;
                const bool search = m_nativeTask == ReadSearchSession;
                if (!error.isEmpty()) {
                    finishNative(error);
                    return;
                }
                // Перенесённые cookies сохраняем и сразу запускаем сбор с новой сессией.
                saveCookies(slug, decodeCookies(values));
                m_store.setValue("weblogin-local/" + slug, profileId);
                m_store.sync();
                m_backgroundMode = true;
                m_replaceCookies = true;
                m_nativeTask = Idle;
                setBusy(false);
                emit sessionsChanged();
                startCollection(m_localImportUrl, search);
            });
}
WebLoginManager::~WebLoginManager()
{
    if (m_cookieImporter)
        m_cookieImporter->cancel();
    if (m_external)
        m_external->stop();
}

QString WebLoginManager::normalizeService(const QString& service)
{
    const QString value = service.trimmed().toLower();
    if (value == QStringLiteral("vk") || value == QStringLiteral("вк"))
        return QStringLiteral("vk");
    if (value == QStringLiteral("yandex") || value == QStringLiteral("яндекс") ||
        value == QStringLiteral("яндекс музыка") || value == QStringLiteral("yandex music"))
        return QStringLiteral("yandex");
    return {};
}
QString WebLoginManager::findExecutableIn(const QString& name, const QStringList& dirs)
{
    for (const QString& dir : dirs) {
        const QFileInfo candidate(QDir(dir).filePath(name));
        if (candidate.isFile() && candidate.isExecutable())
            return candidate.absoluteFilePath();
    }
    return {};
}
QStringList WebLoginManager::detectBrowserExecutables()
{
    QStringList names = {QStringLiteral("brave"),
                         QStringLiteral("chromium"),
                         QStringLiteral("chromium-browser"),
                         QStringLiteral("google-chrome"),
                         QStringLiteral("google-chrome-stable"),
                         QStringLiteral("microsoft-edge"),
                         QStringLiteral("microsoft-edge-stable"),
                         QStringLiteral("yandex-browser")};
    QStringList dirs = QString(qgetenv("PATH")).split(QDir::listSeparator(), Qt::SkipEmptyParts);
#ifdef Q_OS_WIN
    names << QStringLiteral("brave.exe") << QStringLiteral("chrome.exe") << QStringLiteral("msedge.exe") << QStringLiteral("browser.exe");
    const QStringList roots = {qEnvironmentVariable("ProgramFiles"), qEnvironmentVariable("ProgramFiles(x86)"),
                               qEnvironmentVariable("LocalAppData")};
    for (const QString& root : roots)
        if (!root.isEmpty())
            dirs << root + QStringLiteral("/BraveSoftware/Brave-Browser/Application")
                 << root + QStringLiteral("/Google/Chrome/Application")
                 << root + QStringLiteral("/Microsoft/Edge/Application")
                 << root + QStringLiteral("/Yandex/YandexBrowser/Application");
#endif
    QStringList found;
    for (const QString& name : names) {
        const QString path = findExecutableIn(name, dirs);
        if (!path.isEmpty() && !found.contains(path))
            found << path;
    }
    for (const QString& fixed : QStringList{QStringLiteral("/opt/yandex/browser/yandex-browser"),
                                            QStringLiteral("/opt/microsoft/msedge/msedge")})
        if (QFileInfo::exists(fixed) && !found.contains(fixed))
            found << fixed;
    return found;
}
QStringList WebLoginManager::availableBrowsers()
{
    if (m_browsers.isEmpty())
        m_browsers = detectBrowserExecutables();
    QStringList result;
    for (const QString& path : m_browsers)
        result << QStringLiteral("%1 (%2)").arg(QFileInfo(path).completeBaseName(), path);
    return result;
}

QJsonArray WebLoginManager::encodeCookies(const QList<StoredCookie>& cookies)
{
    QJsonArray array;
    for (const auto& cookie : cookies)
        array.append(QJsonObject{{"name", cookie.name},
                                 {"value", cookie.value},
                                 {"domain", cookie.domain},
                                 {"path", cookie.path},
                                 {"expires", QString::number(cookie.expires)},
                                 {"secure", cookie.secure},
                                 {"httpOnly", cookie.httpOnly}});
    return array;
}
QList<WebLoginManager::StoredCookie> WebLoginManager::decodeCookies(const QJsonArray& array)
{
    QList<StoredCookie> cookies;
    for (const auto& value : array) {
        const auto object = value.toObject();
        StoredCookie cookie;
        cookie.name = object.value(QStringLiteral("name")).toString();
        cookie.value = object.value(QStringLiteral("value")).toString();
        cookie.domain = object.value(QStringLiteral("domain")).toString();
        cookie.path = object.value(QStringLiteral("path")).toString(QStringLiteral("/"));
        cookie.expires = object.value(QStringLiteral("expires")).toVariant().toLongLong();
        cookie.secure = object.value(QStringLiteral("secure")).toBool();
        cookie.httpOnly = object.value(QStringLiteral("httpOnly")).toBool();
        if (!cookie.name.isEmpty() && !cookie.domain.isEmpty())
            cookies.append(cookie);
    }
    return cookies;
}
void WebLoginManager::saveCookies(const QString& service, const QList<StoredCookie>& cookies)
{
    if (normalizeService(service).isEmpty())
        return;
    m_store.setValue(QStringLiteral("weblogin/%1").arg(normalizeService(service)),
                     QJsonDocument(encodeCookies(cookies)).toJson(QJsonDocument::Compact));
    m_store.sync();
    // Cookies — чувствительные данные, закрываем файл от чтения другими пользователями.
    QFile::setPermissions(m_store.fileName(), QFileDevice::ReadOwner | QFileDevice::WriteOwner);
}
QList<WebLoginManager::StoredCookie> WebLoginManager::storedCookies(const QString& service) const
{
    const auto raw = m_store.value(QStringLiteral("weblogin/%1").arg(normalizeService(service))).toByteArray();
    return decodeCookies(QJsonDocument::fromJson(raw).array());
}
bool WebLoginManager::hasSessionCookie(const QList<StoredCookie>& cookies, const QString& service)
{
    const QString slug = normalizeService(service);
    if (slug.isEmpty())
        return false;
    // Имена сессионных cookies: у VK — remixsid, у Яндекса — Session_id.
    const QString sessionName =
        slug == QStringLiteral("vk") ? QStringLiteral("remixsid") : QStringLiteral("Session_id");
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    for (const auto& cookie : cookies) {
        if ((cookie.name != sessionName && !(slug == "vk" && cookie.name == "remixsid6")) || cookie.value.isEmpty())
            continue;
        if (cookie.expires > 0 && cookie.expires <= now)
            continue;
        if (slug == QStringLiteral("vk")
                ? (domainMatches(cookie.domain, QStringLiteral("vk.com")) ||
                   domainMatches(cookie.domain, QStringLiteral("vk.ru")))
                : serviceForHost(cookie.domain.startsWith('.') ? cookie.domain.mid(1) : cookie.domain) ==
                      QStringLiteral("yandex"))
            return true;
    }
    return false;
}
bool WebLoginManager::hasSession(QString service) const
{
    return hasSessionCookie(storedCookies(service), service);
}

QList<QNetworkCookie> WebLoginManager::cookiesFor(QString service)
{
    QList<QNetworkCookie> result;
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    for (const auto& cookie : storedCookies(service)) {
        if (cookie.expires > 0 && cookie.expires <= now)
            continue;
        QNetworkCookie network(cookie.name.toUtf8(), cookie.value.toUtf8());
        network.setDomain(cookie.domain);
        network.setPath(cookie.path);
        network.setSecure(cookie.secure);
        network.setHttpOnly(cookie.httpOnly);
        if (cookie.expires > 0)
            network.setExpirationDate(QDateTime::fromSecsSinceEpoch(cookie.expires));
        result << network;
    }
    return result;
}
QString WebLoginManager::cookieHeader(const QString& service, const QUrl& origin)
{
    const QString host = origin.host().toLower();
    const bool secureTransport = origin.scheme() == QStringLiteral("https");
    QStringList pairs;
    const qint64 now = QDateTime::currentSecsSinceEpoch();
    for (const auto& cookie : storedCookies(service)) {
        if (cookie.expires > 0 && cookie.expires <= now)
            continue;
        if (cookie.secure && !secureTransport)
            continue;
        if (!domainMatches(host, cookie.domain))
            continue;
        // Проверяем соответствие пути по правилам RFC 6265 (учёт границ сегмента).
        const QString path = cookie.path.isEmpty() ? QStringLiteral("/") : cookie.path;
        const QString requestPath = origin.path().isEmpty() ? QStringLiteral("/") : origin.path();
        if (!requestPath.startsWith(path) ||
            (requestPath.size() > path.size() && !path.endsWith('/') && requestPath.at(path.size()) != '/'))
            continue;
        pairs << cookie.name + QLatin1Char('=') + cookie.value;
    }
    return pairs.join(QStringLiteral("; "));
}

QString WebLoginManager::exportNetscape(QString service)
{
    const auto cookies = storedCookies(service);
    if (cookies.isEmpty())
        return {};
    // Формат Netscape cookie file, который принимает yt-dlp.
    const QString path = QDir(m_cookieTemp.path()).filePath(normalizeService(service) + "-cookies.txt");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return {};
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    QByteArray output = "# Netscape HTTP Cookie File\n# https://curl.se/docs/http-cookies.html\n";
    for (const auto& cookie : cookies) {
        QString domain = cookie.domain;
        const bool hostOnly = !domain.startsWith(QLatin1Char('.'));
        if (cookie.httpOnly)
            domain = QStringLiteral("#HttpOnly_") + domain;
        output += (domain + QLatin1Char('\t') + (hostOnly ? QStringLiteral("FALSE") : QStringLiteral("TRUE")) +
                   QLatin1Char('\t') + (cookie.path.isEmpty() ? QStringLiteral("/") : cookie.path) + QLatin1Char('\t') +
                   (cookie.secure ? QStringLiteral("TRUE") : QStringLiteral("FALSE")) + QLatin1Char('\t') +
                   QString::number(cookie.expires) + QLatin1Char('\t') + cookie.name + QLatin1Char('\t') +
                   cookie.value + QLatin1Char('\n'))
                      .toUtf8();
    }
    if (file.write(output) != output.size()) {
        file.remove();
        return {};
    }
    file.close();
    return path;
}

void WebLoginManager::setBusy(bool busy)
{
    if (m_busy == busy)
        return;
    m_busy = busy;
    emit busyChanged();
}
void WebLoginManager::setStatus(const QString& status)
{
    if (m_status == status)
        return;
    m_status = status;
    emit statusChanged();
}

void WebLoginManager::startLogin(QString service, int browserIndex)
{
    if (m_busy)
        return;
    availableBrowsers();
    if (browserIndex >= 0 && browserIndex < m_browsers.size())
        m_selectedBrowser = m_browsers[browserIndex];
    openNative(service);
}
void WebLoginManager::cancel()
{
    if (m_cookieImporter)
        m_cookieImporter->cancel();
    if (m_nativeTask != Idle)
        finishNative("Операция отменена.");
}
void WebLoginManager::logout(QString service)
{
    const QString slug = normalizeService(service);
    if (slug.isEmpty())
        return;
    if (m_nativeTask != Idle && slug == m_nativeSlug)
        finishNative("Выполнен выход из аккаунта.");
    if (m_external && m_external->service() == slug)
        m_external->stop();
#ifdef AURORA_WEBENGINE
    if (m_profiles.contains(slug))
        m_profiles.value(slug)->cookieStore()->deleteAllCookies();
#endif
    m_store.remove("weblogin/" + slug);
    m_store.sync();
    m_store.remove("weblogin-local/" + slug);
    m_store.sync();
    // Удаляем cookies и профили, связанные с сессией сервиса.
    const auto data = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir(data + "/weblogin/" + slug).removeRecursively();
    QDir(data + "/service-browser/" + slug).removeRecursively();
    QDir(data + "/service-browser/local-" + slug).removeRecursively();
    if (!m_profiles.contains(slug))
        QDir(data + "/webprofiles/" + slug).removeRecursively();
    emit sessionsChanged();
}
QString WebLoginManager::serviceUserAgent() const
{
    if (m_externalMode && m_external && !m_external->userAgent().isEmpty())
        return m_external->userAgent();
#ifdef AURORA_WEBENGINE
    if (m_nativeProfile)
        return m_nativeProfile->httpUserAgent();
#endif
    return QStringLiteral("AuroraPlayer/4.0");
}
void WebLoginManager::navigateNative(const QUrl& url)
{
    // Внешний браузер открываем напрямую, встроенному — отправляем запрос на навигацию.
    if (m_externalMode && m_external) {
        const bool replace = m_replaceCookies;
        m_replaceCookies = false;
        m_external->open(m_selectedBrowser, m_nativeSlug, url, encodeCookies(storedCookies(m_nativeSlug)),
                         m_backgroundMode, replace);
    } else
        emit navigateRequested(url);
}
QVariantList WebLoginManager::localBrowserProfiles() const
{
    return m_cookieImporter->availableProfiles();
}
void WebLoginManager::collectFromBrowser(const QString& service, const QUrl& url, const QString& profileId)
{
    if (m_busy) {
        emit collectionReady({}, {}, "Дождись завершения текущего импорта.");
        return;
    }
    const QString slug = normalizeService(service);
    if (slug.isEmpty()) {
        emit collectionReady({}, {}, "Неизвестный музыкальный сервис.");
        return;
    }
    m_localImportUrl =
        url.isEmpty() ? QUrl(slug == "vk" ? "https://vk.ru/audio" : "https://music.yandex.ru/collection") : url;
    if (serviceForHost(m_localImportUrl.host()) != slug || m_localImportUrl.scheme() != "https") {
        emit collectionReady({}, {}, "Нужна ссылка выбранного музыкального сервиса по HTTPS.");
        return;
    }
    m_nativeSlug = slug;
    m_nativeTask = ReadLocalSession;
    setBusy(true);
    setStatus("Ищем существующую сессию в браузере…");
    // Сначала ищем cookies в локальных профилях браузеров.
    m_cookieImporter->read(slug, profileId);
}
void WebLoginManager::searchMusic(const QString& query, bool recommendations)
{
    if (m_busy) {
        emit searchReady({}, "VK занят импортом или загрузкой. Повтори поиск после завершения.");
        return;
    }
    QUrl url("https://vk.ru/audio");
    QUrlQuery args;
    args.addQueryItem("section", recommendations ? "explore" : "search");
    if (!recommendations)
        args.addQueryItem("q", query.trimmed().left(250));
    url.setQuery(args);
    // Если сессия VK уже перенесена, ищем сразу в фоне; иначе сначала читаем cookies.
    if (hasSession("vk") && !m_store.value("weblogin-local/vk").toString().isEmpty()) {
        m_backgroundMode = true;
        startCollection(url, true);
        return;
    }
    m_localImportUrl = url;
    m_nativeSlug = "vk";
    m_nativeTask = ReadSearchSession;
    setBusy(true);
    setStatus("Подключаем поиск VK к сохранённому входу…");
    m_cookieImporter->read("vk", m_store.value("weblogin-local/vk").toString());
}
void WebLoginManager::cancelSearch()
{
    if (m_nativeTask == ReadSearchSession) {
        m_cookieImporter->cancel();
        finishNative("Поиск отменён.");
    } else if (m_nativeTask == Search)
        finishNative("Поиск отменён.");
}
void WebLoginManager::requestVkRecommendations()
{
    if (busy()) {
        emit vkRecommendationsReady({}, "Сначала заверши текущую операцию сервиса.");
        return;
    }
    if (!hasSession("vk")) {
        emit vkRecommendationsReady({}, "Подключи VK Музыку, чтобы получать рекомендации своего аккаунта.");
        return;
    }
    m_backgroundMode = !detectBrowserExecutables().isEmpty();
    startCollection(QUrl("https://vk.ru/audio?section=explore"), true, true);
}

void WebLoginManager::cancelVkRecommendations()
{
    if (m_nativeTask == Recommend)
        finishNative("Обновление рекомендаций VK отменено.");
}
