#include "browserauthmanager.h"

#include "settingsstore.h"
#include "webloginmanager.h"

#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QUrl>

#include <utility>

// Реализация BrowserAuthManager: определение сервиса, открытие входа,
// поиск браузеров и профилей Firefox, формирование аргументов yt-dlp.

namespace
{

// Приводит произвольное название сервиса к каноническому имени (или пустой строке).
QString canonicalService(const QString& service)
{
    const QString value = service.trimmed().toLower();
    if (value == QStringLiteral("youtube") || value == QStringLiteral("ютуб")) {
        return QStringLiteral("YouTube");
    }
    if (value == "soundcloud")
        return "SoundCloud";
    if (value == QStringLiteral("yandex") || value == QStringLiteral("яндекс") ||
        value == QStringLiteral("яндекс музыка") || value == QStringLiteral("yandex music")) {
        return QStringLiteral("Яндекс Музыка");
    }
    if (value == QStringLiteral("vk") || value == QStringLiteral("вк")) {
        return QStringLiteral("VK");
    }
    if (value == QStringLiteral("spotify") || value == QStringLiteral("спотифай")) {
        return QStringLiteral("Spotify");
    }
    return {};
}

} // namespace

BrowserAuthManager::BrowserAuthManager(SettingsStore* settings, WebLoginManager* webLogin, QObject* parent)
    : QObject(parent), m_settings(settings), m_webLogin(webLogin)
{}

bool BrowserAuthManager::openService(const QString& service)
{
    const QString canonical = canonicalService(service);
    // VK и Яндекс Музыку открываем через встроенный веб-логин, если он доступен.
    if (m_webLogin && (canonical == "VK" || canonical == "Яндекс Музыка")) {
        m_webLogin->openNative(canonical);
        return true;
    }
    QUrl url;
    if (canonical == QStringLiteral("YouTube")) {
        url = QUrl(QStringLiteral("https://www.youtube.com/"));
    } else if (canonical == "SoundCloud") {
        url = QUrl("https://soundcloud.com/");
    } else if (canonical == QStringLiteral("Яндекс Музыка")) {
        url = QUrl(QStringLiteral("https://music.yandex.ru/"));
    } else if (canonical == QStringLiteral("VK")) {
        url = QUrl(QStringLiteral("https://vk.com/"));
    } else if (canonical == QStringLiteral("Spotify")) {
        url = QUrl(QStringLiteral("https://accounts.spotify.com/login"));
    } else {
        setLastError(tr("Неизвестный сервис авторизации: %1").arg(service));
        return false;
    }

    if (!QDesktopServices::openUrl(url)) {
        setLastError(tr("Не удалось открыть браузер для %1.").arg(canonical));
        return false;
    }
    setLastError({});
    return true;
}

void BrowserAuthManager::grantSessionAccess()
{
    if (!m_sessionGranted) {
        m_sessionGranted = true;
        emit sessionGrantedChanged();
    }
    setLastError({});
}

void BrowserAuthManager::revokeSessionAccess()
{
    if (m_sessionGranted) {
        m_sessionGranted = false;
        emit sessionGrantedChanged();
    }
    setLastError({});
}

QStringList BrowserAuthManager::sessionArguments(const QString& service, bool authorized) const
{
    const QString canonical = canonicalService(service);
    if (!authorized || !m_sessionGranted || canonical.isEmpty() || canonical == QStringLiteral("Spotify")) {
        return {};
    }

    // Сначала пробуем отдать cookies в виде файла Netscape (VK/Яндекс).
    if (m_webLogin) {
        const QString slug = canonical == QStringLiteral("VK")              ? QStringLiteral("vk")
                             : canonical == QStringLiteral("Яндекс Музыка") ? QStringLiteral("yandex")
                                                                            : QString();
        if (!slug.isEmpty() && m_webLogin->hasSession(slug)) {
            const QString cookiesFile = m_webLogin->exportNetscape(slug);
            if (!cookiesFile.isEmpty()) {
                return {QStringLiteral("--cookies"), cookiesFile};
            }
        }
    }

    if (canonical == "VK" || canonical == "Яндекс Музыка")
        return {};
    QString browser = m_settings ? m_settings->property("browser").toString().trimmed() : QString();
    if (browser.isEmpty()) {
        browser = QStringLiteral("firefox");
    }
    const QString profile = m_settings ? m_settings->property("browserProfile").toString().trimmed() : QString();
    if (!profile.isEmpty()) {
        browser += QLatin1Char(':') + profile;
    }
    return {QStringLiteral("--cookies-from-browser"), browser};
}

QStringList BrowserAuthManager::detectBrowsers(const QString& home)
{
    QStringList result;
    const auto exists = [&home](const QString& relative) {
        return QFileInfo::exists(home + QLatin1Char('/') + relative);
    };

    if (exists(QStringLiteral(".mozilla/firefox/profiles.ini")) ||
        exists(QStringLiteral("snap/firefox/common/.mozilla/firefox/profiles.ini")) ||
        exists(QStringLiteral(".var/app/org.mozilla.firefox/.mozilla/firefox/profiles.ini"))) {
        result << QStringLiteral("firefox");
    }
    // Пары «имя браузера — путь к его конфигу» для Chromium-подобных браузеров.
    const std::pair<const char*, const char*> probes[] = {
        {"brave", ".config/BraveSoftware/Brave-Browser"},
        {"chromium", ".config/chromium"},
        {"chrome", ".config/google-chrome"},
        {"opera", ".config/opera"},
        {"vivaldi", ".config/vivaldi"},
        {"edge", ".config/microsoft-edge"},
    };
    for (const auto& [name, relative] : probes) {
        if (exists(QString::fromLatin1(relative))) {
            result << QString::fromLatin1(name);
        }
    }
    return result;
}

QString BrowserAuthManager::detectFirefoxProfile(const QString& home)
{
    // Читаем profiles.ini из всех возможных мест установки Firefox.
    const QStringList roots = {
        home + QStringLiteral("/.mozilla/firefox/profiles.ini"),
        home + QStringLiteral("/snap/firefox/common/.mozilla/firefox/profiles.ini"),
        home + QStringLiteral("/.var/app/org.mozilla.firefox/.mozilla/firefox/profiles.ini"),
    };
    QStringList names;
    QString defaultName;
    for (const QString& path : roots) {
        if (!QFileInfo::exists(path)) {
            continue;
        }
        QSettings ini(path, QSettings::IniFormat);
        const QStringList groups = ini.childGroups();
        for (const QString& group : groups) {
            if (!group.startsWith(QStringLiteral("Profile"))) {
                continue;
            }
            ini.beginGroup(group);
            const QString name = ini.value(QStringLiteral("Name")).toString();
            if (!name.isEmpty()) {
                names << name;
                if (ini.value(QStringLiteral("Default"), 0).toInt() == 1) {
                    defaultName = name;
                }
            }
            ini.endGroup();
        }
    }
    if (!defaultName.isEmpty()) {
        return defaultName;
    }
    return names.size() == 1 ? names.first() : QString();
}

QStringList BrowserAuthManager::detectedBrowsers() const
{
    return detectBrowsers(QDir::homePath());
}

QString BrowserAuthManager::suggestBrowser() const
{
    const QStringList found = detectedBrowsers();
    return found.isEmpty() ? QString() : found.first();
}

QString BrowserAuthManager::suggestFirefoxProfile() const
{
    return detectFirefoxProfile(QDir::homePath());
}

QString BrowserAuthManager::consentText(const QString& service) const
{
    const QString canonical = canonicalService(service);
    if (canonical == QStringLiteral("Spotify")) {
        return tr("Spotify использует собственный вход spotdl. Aurora не передаёт cookies браузера для Spotify "
                  "и не получает ваш пароль.");
    }
    if (canonical.isEmpty()) {
        return tr("Выберите поддерживаемый сервис. Aurora не читает и не хранит пароли или cookies.");
    }
    return tr("После входа в %1 в браузере вы можете разрешить yt-dlp использовать эту сессию. "
              "Доступ предоставляется только для выбранных загрузок и может быть отозван в любой момент. "
              "Aurora не читает, не копирует и не хранит пароли или cookies; к ним обращается yt-dlp.")
        .arg(canonical);
}

void BrowserAuthManager::setLastError(const QString& lastError)
{
    if (m_lastError == lastError) {
        return;
    }
    m_lastError = lastError;
    emit lastErrorChanged();
}
