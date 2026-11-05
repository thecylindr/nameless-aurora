#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QVector>

// Менеджер очереди загрузок: запускает yt-dlp/spotdl и «нативные» загрузки
// (VK/Яндекс Музыка), следит за прогрессом, ошибками и импортом файлов в библиотеку.
class QNetworkAccessManager;
class BrowserAuthManager;
class DependencyManager;
class ImportManager;
class LibraryModel;
class SettingsStore;

class DownloadManager : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(int activeCount READ activeCount NOTIFY activeCountChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY summaryChanged)
    Q_PROPERTY(int fileTotal READ fileTotal NOTIFY summaryChanged)
    Q_PROPERTY(int filesReady READ filesReady NOTIFY summaryChanged)
    Q_PROPERTY(int failedCount READ failedCount NOTIFY summaryChanged)
    Q_PROPERTY(int succeededCount READ succeededCount NOTIFY summaryChanged)
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY summaryChanged)
    Q_PROPERTY(QString currentService READ currentService NOTIFY summaryChanged)
    Q_PROPERTY(int firstFailedTrack READ firstFailedTrack NOTIFY summaryChanged)

  public:
    enum Role
    {
        JobIdRole = Qt::UserRole + 1,
        TitleRole,
        UrlRole,
        StatusRole,
        ProgressRole,
        ErrorRole,
        ServiceRole,
        TrackIdRole
    };
    Q_ENUM(Role)

    explicit DownloadManager(SettingsStore* settings, ImportManager* importer, DependencyManager* dependencies,
                             BrowserAuthManager* auth, QObject* parent = nullptr,
                             QNetworkAccessManager* transport = nullptr);
    ~DownloadManager() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool busy() const
    {
        return m_busy;
    }
    double progress() const
    {
        return m_progress;
    }
    QString statusText() const
    {
        return m_statusText;
    }
    QString lastError() const
    {
        return m_lastError;
    }
    int activeCount() const
    {
        return m_activeCount;
    }
    int totalCount() const
    {
        return m_jobs.size();
    }
    int fileTotal() const
    {
        return m_fileBatch.size();
    }
    int filesReady() const;
    // Регистрирует треки в пакете загрузки (нужно для подсчёта итогов и статусов).
    void registerTracks(const QList<int>& ids);
    int failedCount() const;
    int succeededCount() const;
    QString currentTitle() const;
    QString currentService() const;
    int firstFailedTrack() const;

    // Добавляет ссылки (YouTube/Яндекс Музыка/VK/Spotify) в очередь и запускает обработку.
    Q_INVOKABLE void enqueue(const QStringList& urls, const QString& quality, bool authorized = false,
                             const QStringList& titles = {});
    // Ставит в очередь «нативные» треки VK/Яндекс Музыки с уже известными метаданными.
    Q_INVOKABLE void enqueueNative(const QVariantList& tracks, const QString& quality);
    Q_INVOKABLE void cancelJob(int id);
    Q_INVOKABLE void retryJob(int id);
    Q_INVOKABLE void clearFinished();
    Q_INVOKABLE void cancelAll();
    Q_INVOKABLE void retryFailed();
    Q_INVOKABLE void openDiagnostics();
    Q_INVOKABLE QVariantMap trackState(int trackId) const;
    Q_INVOKABLE void downloadTrack(int trackId);
    // Скачивает все треки текущей выборки, у которых файл отсутствует в библиотеке.
    Q_INVOKABLE void downloadMissing();
    // Пытается скачать конкретный трек по найденному кандидату из другого источника.
    Q_INVOKABLE bool enqueueAlternative(int trackId, const QVariantMap& candidate, const QString& quality);
    // Определяет сервис по URL (YouTube, VK, Spotify, Zaycev.net и т. д.).
    Q_INVOKABLE QString serviceForUrl(const QString& url) const;

  signals:
    void busyChanged();
    void progressChanged();
    void statusTextChanged();
    void lastErrorChanged();
    void activeCountChanged();
    void summaryChanged();
    void trackTransferFinished(int trackId, bool success, const QString& source, const QString& error);

  private:
    struct Job;

    int indexOfJob(int id) const;
    bool isActive(const Job* job) const;
    int concurrencyLimit() const;
    void startQueued();
    void startJob(Job* job);
    void startNative(Job* job);
    void downloadNative(Job* job, const QUrl& url);
    void convertNative(Job* job, const QString& input, bool remote);
    void cleanupNative(Job* job);
    bool validateNativeFile(Job* job, const QString& path, QString* error);
    static bool validateAudioFile(const QString& path, double expected, const QString& ffprobe, const QString& ffmpeg,
                                  QString* error);
    void readProcessOutput(Job* job, bool errorOutput);
    void finishCancelled(Job* job);
    void finishFailed(Job* job, const QString& message);
    void recordFailure(const Job* job, const QString& message);
    void finishSucceeded(Job* job);
    bool importDownloadedFiles(Job* job, QString* errorMessage);
    void setJobStatus(Job* job, const QString& status);
    void setJobProgress(Job* job, double progress);
    void removeTempDir(Job* job);
    void updateAggregate();
    void setStatusText(const QString& statusText);
    void setLastError(const QString& lastError);

    SettingsStore* m_settings;
    ImportManager* m_importer;
    DependencyManager* m_dependencies;
    BrowserAuthManager* m_auth;
    LibraryModel* m_library;
    QNetworkAccessManager* m_network;
    QVector<Job*> m_jobs;
    QHash<int, bool> m_fileBatch;
    int m_nextJobId = 1;
    bool m_busy = false;
    double m_progress = 0.0;
    QString m_statusText;
    QString m_lastError;
    int m_activeCount = 0;
    QHash<QString, int> m_serviceFailures;
    bool m_preparingTools = false;
};
