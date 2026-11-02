#include "browserauthmanager.h"

#include "settingsstore.h"

#include <QDesktopServices>
#include <QUrl>

namespace {

QString canonicalService(const QString &service)
{
    const QString value = service.trimmed().toLower();
    if (value == QStringLiteral("youtube") || value == QStringLiteral("ютуб")) {
        return QStringLiteral("YouTube");
    }
    if (value == QStringLiteral("yandex") || value == QStringLiteral("яндекс")
        || value == QStringLiteral("яндекс музыка") || value == QStringLiteral("yandex music")) {
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

}

BrowserAuthManager::BrowserAuthManager(SettingsStore *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
}

bool BrowserAuthManager::openService(const QString &service)
{
    const QString canonical = canonicalService(service);
    QUrl url;
    if (canonical == QStringLiteral("YouTube")) {
        url = QUrl(QStringLiteral("https://www.youtube.com/"));
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

QStringList BrowserAuthManager::sessionArguments(const QString &service, bool authorized) const
{
    const QString canonical = canonicalService(service);
    if (!authorized || !m_sessionGranted || canonical.isEmpty() || canonical == QStringLiteral("Spotify")) {
        return {};
    }

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

QString BrowserAuthManager::consentText(const QString &service) const
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

void BrowserAuthManager::setLastError(const QString &lastError)
{
    if (m_lastError == lastError) {
        return;
    }
    m_lastError = lastError;
    emit lastErrorChanged();
}
