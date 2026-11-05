#pragma once
// OAuth-авторизация VK и Яндекс Музыки: локальный callback-сервер, обмен кода
// на токен, сохранение и обновление токенов доступа.
#include <QObject>
#include <QNetworkAccessManager>
class SettingsStore;
class QTcpServer;
class QTcpSocket;
class QTimer;

class OAuthManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool vkLoggedIn READ vkLoggedIn NOTIFY accountsChanged)
    Q_PROPERTY(bool yandexLoggedIn READ yandexLoggedIn NOTIFY accountsChanged)
  public:
    explicit OAuthManager(SettingsStore* settings, QObject* parent = nullptr);
    bool busy() const
    {
        return m_busy;
    }
    QString lastError() const
    {
        return m_lastError;
    }
    // Есть ли сохранённый токен VK.
    bool vkLoggedIn() const;
    // Есть ли сохранённый токен Яндекс Музыки.
    bool yandexLoggedIn() const;
    // Возвращает токен VK (пустая строка — не авторизован).
    QString vkToken() const;
    // Возвращает токен Яндекса, при необходимости обновляя его по refresh_token.
    QString yandexToken();
    // Принудительно обновляет токен Яндекса по refresh_token (синхронно).
    bool refreshYandexToken();
    // Запускает OAuth-вход: открывает браузер и ждёт redirect на локальный порт.
    Q_INVOKABLE void login(const QString& service);
    // Удаляет сохранённый токен сервиса.
    Q_INVOKABLE void logout(const QString& service);
    // Сохраняет вручную вставленный токен доступа.
    Q_INVOKABLE void saveToken(const QString& service, const QString& token);
    // Возвращает пошаговую инструкцию регистрации приложения для сервиса.
    Q_INVOKABLE QString serviceHint(const QString& service) const;
    // URL авторизации VK с заданным client_id.
    static QString vkAuthorizeUrl(const QString& clientId);
    // URL авторизации Яндекса с заданным client_id.
    static QString yandexAuthorizeUrl(const QString& clientId);
    // Разбирает query-строку redirect-ответа на код/токен/ошибку.
    static bool parseRedirectQuery(const QString& raw, QString* code, QString* accessToken, qint64* expiresIn,
                                   QString* state, QString* error);
    // Применяет JSON-ответ обмена кода на токен к настройкам.
    static bool applyTokenJson(SettingsStore* settings, const QString& service, const QByteArray& json,
                               QString* message);
  signals:
    // Изменился флаг занятости.
    void busyChanged();
    // Изменился текст последней ошибки.
    void lastErrorChanged();
    // Изменился набор авторизованных аккаунтов.
    void accountsChanged();
    // Вход завершён (успешно или с ошибкой).
    void loginDone(const QString& service, bool ok, const QString& message);

  private:
    // Готовит состояние к новой операции входа.
    void begin(const QString& service);
    // Освобождает callback-сервер и таймер, снимает флаг занятости.
    void cleanup();
    // Завершает вход ошибкой.
    void fail(const QString& service, const QString& message);
    // Завершает вход успехом.
    void succeed(const QString& service, const QString& message);
    // Обрабатывает пришедший на локальный порт redirect.
    void handleCallback(const QString& requestLine);
    // Обменивает код авторизации на токен доступа.
    void exchangeCode(const QString& service, const QString& code);
    SettingsStore* m_settings;
    QNetworkAccessManager m_network;
    QTcpServer* m_server = nullptr;
    QTimer* m_timer = nullptr;
    QString m_service;
    bool m_busy = false;
    QString m_lastError;
};
