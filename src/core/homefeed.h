#pragma once

#include <QDate>
#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QSet>
#include <QVariantList>

class LibraryModel;
class OAuthManager;
class PlaybackController;
class WebLoginManager;
class AudioSources;
struct Track;

// Подборки главной и отдельная выдача «Моей волны» из аккаунта Яндекса.
class HomeFeed : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList sections READ sections NOTIFY sectionsChanged)
    Q_PROPERTY(QVariantList yandexTracks READ yandexTracks NOTIFY yandexChanged)
    Q_PROPERTY(bool yandexConnected READ yandexConnected NOTIFY yandexChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY yandexChanged)
    Q_PROPERTY(QString status READ status NOTIFY yandexChanged)
    Q_PROPERTY(QVariantList vkTracks READ vkTracks NOTIFY vkChanged)
    Q_PROPERTY(bool vkConnected READ vkConnected NOTIFY vkChanged)
    Q_PROPERTY(bool vkBusy READ vkBusy NOTIFY vkChanged)
    Q_PROPERTY(QString vkStatus READ vkStatus NOTIFY vkChanged)
    Q_PROPERTY(QVariantMap preferences READ preferences WRITE setPreferences NOTIFY preferencesChanged)
    Q_PROPERTY(QString playbackStatus READ playbackStatus NOTIFY playbackChanged)
    Q_PROPERTY(int resolvingId READ resolvingId NOTIFY playbackChanged)
    Q_PROPERTY(QString resolvingUrl READ resolvingUrl NOTIFY playbackChanged)

  public:
    HomeFeed(LibraryModel* library, OAuthManager* accounts, WebLoginManager* web, PlaybackController* player,
             QObject* parent = nullptr, QNetworkAccessManager* transport = nullptr, AudioSources* preview = nullptr);

    QVariantList sections() const
    {
        return m_sections;
    }
    QVariantList yandexTracks() const
    {
        return m_yandexTracks;
    }
    bool yandexConnected() const;
    bool busy() const
    {
        return m_busy;
    }
    QString status() const
    {
        return m_status;
    }
    QVariantList vkTracks() const
    {
        return m_vkTracks;
    }
    bool vkConnected() const;
    bool vkBusy() const
    {
        return m_vkBusy;
    }
    QString vkStatus() const
    {
        return m_vkStatus;
    }
    QVariantMap preferences() const
    {
        return m_preferences;
    }
    void setPreferences(const QVariantMap& preferences);
    QString playbackStatus() const
    {
        return m_playbackStatus;
    }
    int resolvingId() const
    {
        return m_resolvingId;
    }
    QString resolvingUrl() const
    {
        return m_resolvingUrl;
    }

    // Дата передаётся явно, чтобы ротацию можно было проверить без изменения системных часов.
    static QVariantList buildSections(LibraryModel* library, const QDate& date, const QVariantMap& options = {});
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void refreshYandex(bool force = false);
    Q_INVOKABLE void playMix(const QVariantMap& mix, int trackId = 0);
    Q_INVOKABLE void playWave();
    Q_INVOKABLE void playYandex(int index = 0);
    Q_INVOKABLE void refreshVk(bool force = false);
    Q_INVOKABLE void playVk(int index = 0);
    Q_INVOKABLE void playTrack(int trackId);
    Q_INVOKABLE void playLibraryTrack(int trackId);
    Q_INVOKABLE void findSource(int trackId);

  signals:
    void sectionsChanged();
    void yandexChanged();
    void sourceRequested(int trackId);
    void vkChanged();
    void preferencesChanged();
    void playbackChanged();

  private:
    void fetchYandex();
    void receiveYandex(const QVector<Track>& tracks, const QString& error);
    void resolveStream(const QVariantMap& candidate);
    void accountChanged();
    void pumpProviders();
    void receiveVk(const QVector<Track>& tracks, const QString& error);
    void finishStream(const QVariantMap& candidate, const QString& error);
    void cancelResolution();
    void startQueue(const QVariantList& tracks, int index, const QString& provider);
    void startLookup();
    void tryAlternative();
    void failAttempt(const QString& error);
    void receiveAlternatives();
    void playbackStarted();

    LibraryModel* m_library;
    OAuthManager* m_accounts;
    WebLoginManager* m_web;
    PlaybackController* m_player;
    QNetworkAccessManager* m_network;
    AudioSources* m_preview;
    AudioSources* m_lookup = nullptr;
    QVariantMap m_preferences;
    QVariantList m_sections, m_yandexTracks;
    QStringList m_recentYandex;
    QTimer m_refreshTimer;
    QDate m_date;
    qint64 m_fetchedAt = 0;
    int m_generation = 0;
    int m_mediaRequest = 0;
    bool m_busy = false;
    bool m_playWhenReady = false;
    bool m_webConnected = false;
    QVariantMap m_candidate;
    QString m_status;
    QVariantList m_vkTracks;
    qint64 m_vkFetchedAt = 0;
    bool m_vkBusy = false;
    bool m_vkWebConnected = false;
    bool m_vkWaiting = false;
    bool m_yandexWaiting = false;
    bool m_playVkWhenReady = false;
    bool m_previewPending = false;
    QString m_queueProvider;
    QString m_playbackStatus, m_resolvingUrl;
    QString m_vkStatus;
    int m_resolvingId = 0;
    QTimer m_resolveTimer;
    QTimer m_sourceGraceTimer;
    QTimer m_bufferTimer;
    QVariantList m_alternatives;
    QVariantMap m_selectedAlternative;
    QSet<QString> m_attemptedUrls;
    QHash<QString, int> m_sourceAttempts;
    bool m_lookupStarted = false;
    bool m_fallbackAllowed = false;
    bool m_playbackAttempt = false;
    bool m_fallbackAttempt = false;
    bool m_retryScheduled = false;
    bool m_startingStream = false;
    int m_resolutionGeneration = 0;
    QString m_resolutionError;
};
