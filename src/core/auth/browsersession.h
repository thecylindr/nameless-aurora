#pragma once
#include <QElapsedTimer>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QProcess>
#include <QSet>
#include <QTimer>
#include <QUrl>
#include <functional>
// Управляет выделенным окном Chromium через протокол CDP (навигация, cookies, аудио, API).
class CdpClient;

// Окно Chromium, принадлежащее приложению: все страницы, скрипты и cookies
// остаются в изолированном профиле Aurora.
class BrowserSession : public QObject
{
    Q_OBJECT
  public:
    // allocatePort позволяет переопределить выбор порта (для тестов).
    explicit BrowserSession(QObject* parent = nullptr, QNetworkAccessManager* transport = nullptr,
                            CdpClient* protocol = nullptr, std::function<int()> allocatePort = {});
    ~BrowserSession() override;
    // Запускает (или переиспользует) окно браузера и открывает сервис с переносом cookies.
    void open(const QString& browser, const QString& service, const QUrl& url, const QJsonArray& cookies,
              bool background = false, bool replaceCookies = false);
    // Выполняет JS-скрипт в контексте страницы сервиса.
    void evaluate(int requestId, const QString& script, bool userGesture = false);
    // Запрашивает текущие cookies страницы.
    void requestCookies();
    // Выполняет fetch на странице и возвращает JSON-ответ (для API Яндекс Музыки).
    void fetchJson(int requestId, const QUrl& url, const QJsonArray& trackIds = {});
    // Проверяет, является ли URL вызовом API коллекции Яндекс Музыки.
    static bool isCollectionApi(const QUrl& url);
    // Собирает диагностику протокола CDP для отчёта импорта.
    QJsonObject protocolDiagnostic() const;
    // Скрывает окно и приостанавливает воспроизведение до следующей операции.
    void hide();
    // Останавливает браузер и освобождает ресурсы.
    void stop();
    QString userAgent() const
    {
        return m_userAgent;
    }
    QString service() const
    {
        return m_service;
    }
    // Преобразует cookies в формат, принимаемый CDP (Network.setCookie).
    static QJsonArray protocolCookies(const QJsonArray& cookies);
    // Выбирает подходящую страницу (WebSocket-цель) среди целей отладки браузера.
    static QUrl choosePage(const QJsonArray& pages, const QString& service, int port);
    // Преобразует код сетевой ошибки Chromium в понятный текст.
    static QString failureText(const QString& error);
  signals:
    // Страница сервиса открыта (или сообщено об ошибке).
    void loaded(const QUrl& url, bool ok, const QString& error);
    // Результат выполнения скрипта из evaluate().
    void evaluated(int requestId, const QVariant& result);
    // Получены cookies страницы по запросу requestCookies().
    void cookiesReceived(const QString& service, const QJsonArray& cookies);
    // Обнаружена прямая ссылка на аудиофайл.
    void audioRequested(const QUrl& url);
    // Получено тело ответа API (для коллекций Яндекса).
    void apiReceived(const QUrl& url, const QJsonValue& payload);
    // Завершён fetchJson: статус и полезная нагрузка.
    void apiFetched(int requestId, const QUrl& url, const QJsonValue& payload, int status);
    // Операция завершилась ошибкой.
    void failed(const QString& error);

  private:
    // Типы команд, отслеживаемых по id для обработки ответов CDP.
    enum Command
    {
        SeedCookie,
        ClearCookies,
        Cookies,
        Evaluate,
        Agent,
        Navigate,
        NavigateFallback,
        NavigateProbe,
        ApiBody,
        ApiFetch,
        WindowShow,
        WindowHide,
        Ignore
    };
    // Периодически ищет отладочный порт браузера и подключает CDP.
    void poll();
    // Включает домены Page/Network и засевает cookies после запуска.
    void initializePage();
    // Переносит cookies в окно браузера по одной через Network.setCookie.
    void seedCookies();
    // Учитывает результат засева одной cookie.
    void cookieFinished(int index, bool accepted);
    // Читает версию браузера и запускает навигацию (с таймаутом).
    void readAgent();
    // Отправляет Page.navigate и планирует проверку готовности страницы.
    void navigate();
    // Запасной переход через JS, если Page.navigate отклонён.
    void fallbackNavigation();
    // Опрашивает страницу, чтобы подтвердить завершение навигации.
    void probeNavigation(int generation, int navigation);
    // Фиксирует успешную загрузку страницы сервиса.
    void navigationReady(const QUrl& url);
    // Показывает или сворачивает окно браузера.
    void window(bool visible);
    // Обрабатывает ответы CDP-команд по их идентификатору.
    void onResult(int id, const QJsonObject& result);
    // Обрабатывает события CDP (навигация, сеть, аудио).
    void onEvent(const QString& method, const QJsonObject& params);
    // Отправляет CDP-команду и сопоставляет её с внутренним типом.
    int command(const QString& method, const QJsonObject& params, Command kind, int requestId = 0);
    // Останавливает работу и сообщает об ошибке.
    void fail(const QString& error);
    QProcess* m_browser = nullptr;
    QNetworkAccessManager* m_network;
    CdpClient* m_client;
    std::function<int()> m_allocatePort;
    QTimer m_poll, m_idle;
    QElapsedTimer m_elapsed;
    QHash<int, QPair<Command, int>> m_commands;
    QHash<int, QUrl> m_apiCommands;
    QHash<QString, QUrl> m_apiRequests;
    QJsonObject m_apiHeaders;
    QJsonArray m_acceptedCookies;
    QString m_protocolStage, m_protocolError;
    int m_protocolCode = 0, m_navigation = 0;
    int m_fallbackAttempts = 0;
    bool m_navigationFallback = false, m_navigationReady = false;
    bool m_navigationProbePending = false;
    bool m_playbackConfirmed = false;
    int m_seedPending = 0, m_seedRejected = 0;
    QString m_service, m_executable, m_userAgent, m_mainFrame;
    QUrl m_target, m_loaded;
    QJsonArray m_cookies;
    QSet<QString> m_documents, m_audioCandidates;
    QSet<int> m_playEvaluations;
    int m_port = 0, m_generation = 0;
    bool m_stopping = false, m_hidden = false, m_inFlight = false, m_initialized = false, m_cookiePending = false;
    bool m_background = false;
};
