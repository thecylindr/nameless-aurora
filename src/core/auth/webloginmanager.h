#pragma once
#include <QObject>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QProcess>
#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QSet>
#include <QUrl>
#include <QQuickWebEngineProfile>
#include <QElapsedTimer>
#include <QTemporaryDir>
#include <QQueue>
#include <functional>
#include "../data/libraryrepository.h"
// Координация веб-входа и импорта: хранение cookies сессий, запуск внешнего
// или встроенного браузера, сбор плейлистов, поиск VK и получение аудио.
class SettingsStore;
class QTimer;
class CdpClient;
class BrowserSession;
class BrowserCookieImporter;

class WebLoginManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool awaitingLogin READ awaitingLogin NOTIFY nativeBrowserChanged)
    Q_PROPERTY(QQuickWebEngineProfile* nativeProfile READ nativeProfile NOTIFY nativeBrowserChanged)
    Q_PROPERTY(bool browserVisible READ browserVisible NOTIFY nativeBrowserChanged)
    Q_PROPERTY(QUrl pageUrl READ pageUrl NOTIFY nativeBrowserChanged)
    Q_PROPERTY(QString pageError READ pageError NOTIFY nativeBrowserChanged)
    Q_PROPERTY(QString nativeService READ nativeService NOTIFY nativeBrowserChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString importReport READ importReport NOTIFY nativeBrowserChanged)
    Q_PROPERTY(bool vkLoggedIn READ vkLoggedIn NOTIFY sessionsChanged)
    Q_PROPERTY(bool yandexLoggedIn READ yandexLoggedIn NOTIFY sessionsChanged)
  public:
    // Cookie сессии в упрощённом виде для хранения и передачи.
    struct StoredCookie
    {
        QString name, value, domain, path;
        qint64 expires = 0;
        bool secure = false;
        bool httpOnly = false;
    };
    // browserFactory позволяет подменить создание внешнего браузера (для тестов).
    explicit WebLoginManager(SettingsStore* settings, QObject* parent = nullptr,
                             std::function<BrowserSession*(QObject*)> browserFactory = {},
                             BrowserCookieImporter* cookieImporter = nullptr);
    ~WebLoginManager() override;
    bool busy() const
    {
        return m_busy;
    }
    QString status() const
    {
        return m_status;
    }
    QString importReport() const
    {
        return m_importReport;
    }
    // Открывает сохранённый отчёт импорта в системном приложении.
    Q_INVOKABLE void openImportReport();
    bool vkLoggedIn() const
    {
        return hasSession(QStringLiteral("vk"));
    }
    bool yandexLoggedIn() const
    {
        return hasSession(QStringLiteral("yandex"));
    }
    bool awaitingLogin() const
    {
        return m_nativeTask == Login || m_collectLogin;
    }
    QQuickWebEngineProfile* nativeProfile() const
    {
        return m_nativeProfile;
    }
    bool browserVisible() const
    {
        return m_nativeVisible && !m_externalMode;
    }
    QUrl pageUrl() const
    {
        return m_nativeUrl;
    }
    QString pageError() const
    {
        return m_pageError;
    }
    QString nativeService() const
    {
        return m_nativeSlug == "vk" ? "VK" : "Яндекс Музыка";
    }
    // Открывает окно входа сервиса (встроенное или внешний браузер).
    Q_INVOKABLE void openNative(const QString& service);
    // Показывает/скрывает окно сервиса.
    Q_INVOKABLE void setBrowserVisible(bool visible);
    // Получает результат загрузки страницы из браузера.
    Q_INVOKABLE void pageLoaded(const QUrl& url, bool ok, const QString& error = {});
    // Подтверждает завершение входа (кнопка «Я вошёл»).
    Q_INVOKABLE void confirmNativeLogin();
    // Возвращает актуальный User-Agent текущего способа входа.
    QString serviceUserAgent() const;
    // Принимает результат выполнения инжектируемого скрипта.
    Q_INVOKABLE void submitScriptResult(int id, const QVariant& result);
    // Запускает сбор плейлиста по ссылке.
    void collectPlaylist(const QUrl& url);
    // Импорт с переносом cookies из профиля локального браузера.
    void collectFromBrowser(const QString& service, const QUrl& url = {}, const QString& profileId = {});
    // Запускает поиск или рекомендации VK.
    void searchMusic(const QString& query, bool recommendations = false);
    // Отменяет поиск VK.
    void cancelSearch();
    // Список профилей локальных браузеров для импорта.
    Q_INVOKABLE QVariantList localBrowserProfiles() const;
    // Получает прямую аудио-ссылку трека через окно сервиса.
    void resolveMedia(int requestId, const QVariantMap& track);
    // Отменяет получение аудио для указанного запроса.
    void cancelMedia(int requestId);
    // Определяет сервис по хосту (vk/yandex).
    static QString serviceForHost(const QString& host);
    // Разбирает JSON «урожая» треков в список Track.
    static QVector<Track> parseHarvest(const QByteArray& json, const QString& service);
    // Список установленных браузеров для внешнего режима.
    Q_INVOKABLE QStringList availableBrowsers();
    // Начинает вход с выбранным браузером.
    Q_INVOKABLE void startLogin(QString service, int browserIndex);
    // Отменяет текущую операцию.
    Q_INVOKABLE void cancel();
    // Удаляет сессию сервиса (cookies и профиль).
    Q_INVOKABLE void logout(QString service);
    // Есть ли сохранённая действующая сессия сервиса.
    Q_INVOKABLE bool hasSession(QString service) const;
    // Экспортирует cookies в файл формата Netscape для yt-dlp.
    QString exportNetscape(QString service);
    // Возвращает cookies сервиса как QNetworkCookie.
    QList<QNetworkCookie> cookiesFor(QString service);
    // Формирует заголовок Cookie для запроса с учётом домена и пути.
    QString cookieHeader(const QString& service, const QUrl& origin);
    // Сохраняет cookies сессии в QSettings.
    void saveCookies(const QString& service, const QList<StoredCookie>& cookies);
    // Читает сохранённые cookies сервиса.
    QList<StoredCookie> storedCookies(const QString& service) const;
    // Приводит название сервиса к slug (vk/yandex).
    static QString normalizeService(const QString& service);
    // Ищет исполняемый файл по имени в списке директорий.
    static QString findExecutableIn(const QString& name, const QStringList& dirs);
    // Находит установленные Chromium-подобные браузеры.
    static QStringList detectBrowserExecutables();
    // Проверяет наличие актуальной сессионной cookie сервиса.
    static bool hasSessionCookie(const QList<StoredCookie>& cookies, const QString& service);
    // Преобразует QJsonArray в список StoredCookie.
    static QList<StoredCookie> decodeCookies(const QJsonArray& array);
    // Преобразует список StoredCookie в QJsonArray.
    static QJsonArray encodeCookies(const QList<StoredCookie>& cookies);
  signals:
    // Изменилось состояние окна встроенного браузера.
    void nativeBrowserChanged();
    // Просьба к UI открыть URL во встроенном браузере.
    void navigateRequested(const QUrl& url);
    // Просьба к UI выполнить скрипт на странице сервиса.
    void scriptRequested(int id, const QString& script);
    // Сбор плейлиста завершён (треки или ошибка).
    void collectionReady(const QVector<Track>& tracks, const QString& name, const QString& error);
    // Поиск VK завершён (треки или ошибка).
    void searchReady(const QVector<Track>& tracks, const QString& error);
    // Получена прямая аудио-ссылка трека (или ошибка).
    void mediaReady(int requestId, const QUrl& url, const QString& error);
    // Изменился флаг занятости.
    void busyChanged();
    // Изменился текст статуса.
    void statusChanged();
    // Изменился набор сохранённых сессий.
    void sessionsChanged();
    // Вход завершён (успешно или с ошибкой).
    void loginDone(const QString& service, bool ok, const QString& message);

  private:
    // Текущая операция с окном сервиса.
    enum NativeTask
    {
        Idle,
        Login,
        Collect,
        Resolve,
        ReadLocalSession,
        Search,
        ReadSearchSession
    };
    // Запускает сбор плейлиста/поиск с подготовкой состояния.
    void startCollection(const QUrl& url, bool search = false);
    // Разбирает ответ API Яндекс Музыки и накапливает треки.
    void consumeApiPayload(const QUrl& url, const QJsonValue& payload);
    // Ставит запрос к API Яндекса в очередь (с дедупликацией).
    void requestYandex(const QUrl& url, const QJsonArray& ids = {});
    // Отправляет следующий запрос к API Яндекса, если канал свободен.
    void dispatchYandex();
    // Проверяет, собран ли полный плейлист через API, и завершает сбор.
    bool finishApiCollection();
    // Сохраняет отчёт об ошибке импорта в JSON-файл.
    void saveImportReport(const QString& error);
    // Готовит способ входа (внешний браузер или встроенный профиль).
    bool ensureNative(const QString& slug);
    // Периодический шаг: таймауты и инжекция скрипта на страницу.
    void nativeTick();
    // Завершает текущую операцию с результатом или ошибкой.
    void finishNative(const QString& error = {});
    // Берёт следующий трек из очереди получения аудио.
    void nextMedia();
    // Читает скрипт из ресурсов по имени.
    QString nativeScript(const QString& name) const;
    QHash<QString, QQuickWebEngineProfile*> m_profiles;
    QQuickWebEngineProfile* m_nativeProfile = nullptr;
    QString m_nativeSlug, m_pageError, m_collectionName;
    QString m_importReport;
    QJsonObject m_harvestDiagnostic;
    QUrl m_harvestPage;
    int m_expectedTracks = 0;
    bool m_vkEntryRedirected = false;
    QString m_apiPlaylist;
    QStringList m_apiOrder;
    QHash<int, QUrl> m_apiRequests;
    QQueue<QPair<QUrl, QJsonArray>> m_apiQueue;
    QSet<QString> m_apiRequested;
    int m_apiNextId = 0, m_apiResponses = 0, m_apiHttpStatus = 0;
    QUrl m_nativeUrl, m_loadedUrl;
    bool m_nativeVisible = false, m_scriptPending = false, m_mediaArmed = false, m_collectLogin = false;
    NativeTask m_nativeTask = Idle;
    int m_scriptId = 0, m_stableTicks = 0, m_mediaId = 0;
    QElapsedTimer m_nativeElapsed;
    QTimer* m_nativeTimer = nullptr;
    QHash<QString, Track> m_harvest;
    QStringList m_harvestOrder;
    QQueue<QPair<int, QVariantMap>> m_mediaQueue;
    QVariantMap m_requestedTrack;
    QTemporaryDir m_cookieTemp;
    void setBusy(bool busy);
    void setStatus(const QString& status);
    void navigateNative(const QUrl& url);
    SettingsStore* m_settings;
    QSettings m_store;
    BrowserSession* m_external = nullptr;
    std::function<BrowserSession*(QObject*)> m_browserFactory;
    bool m_externalMode = false;
    QString m_selectedBrowser;
    QStringList m_browsers;
    BrowserCookieImporter* m_cookieImporter = nullptr;
    QUrl m_localImportUrl;
    bool m_backgroundMode = false, m_replaceCookies = false;
    bool m_busy = false;
    QString m_status;
};
