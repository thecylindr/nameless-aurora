#pragma once
// Управляет сессией браузера: открывает сервис авторизации, выдаёт и отзывает
// разрешение на доступ к cookies, подбирает браузер и профиль Firefox.

#include <QObject>
#include <QString>
#include <QStringList>

class SettingsStore;
class WebLoginManager;

class BrowserAuthManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool sessionGranted READ sessionGranted NOTIFY sessionGrantedChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

  public:
    // Создаёт менеджер с настройками и (опционально) веб-логином для нативного входа.
    explicit BrowserAuthManager(SettingsStore* settings, WebLoginManager* webLogin = nullptr,
                                QObject* parent = nullptr);

    bool sessionGranted() const
    {
        return m_sessionGranted;
    }
    QString lastError() const
    {
        return m_lastError;
    }

    // Открывает страницу входа выбранного сервиса в системном браузере.
    Q_INVOKABLE bool openService(const QString& service);
    // Разрешает yt-dlp использовать сессию браузера для последующих загрузок.
    Q_INVOKABLE void grantSessionAccess();
    // Отзывает разрешение на использование сессии браузера.
    Q_INVOKABLE void revokeSessionAccess();
    // Возвращает пояснение о том, как Aurora обращается с cookies для сервиса.
    Q_INVOKABLE QString consentText(const QString& service) const;
    // Список найденных в системе браузеров.
    Q_INVOKABLE QStringList detectedBrowsers() const;
    // Предлагает первый найденный браузер для импорта cookies.
    Q_INVOKABLE QString suggestBrowser() const;
    // Предлагает профиль Firefox (по умолчанию или единственный).
    Q_INVOKABLE QString suggestFirefoxProfile() const;

    WebLoginManager* webLogin() const
    {
        return m_webLogin;
    }
    // Формирует аргументы запуска yt-dlp с передачей сессии браузера.
    QStringList sessionArguments(const QString& service, bool authorized) const;

    // Статически ищет установленные браузеры по домашней директории.
    static QStringList detectBrowsers(const QString& home);
    // Определяет имя профиля Firefox из profiles.ini.
    static QString detectFirefoxProfile(const QString& home);

  signals:
    void sessionGrantedChanged();
    void lastErrorChanged();

  private:
    // Устанавливает текст последней ошибки и уведомляет об изменении.
    void setLastError(const QString& lastError);

    SettingsStore* m_settings;
    WebLoginManager* m_webLogin;
    bool m_sessionGranted = false;
    QString m_lastError;
};
