#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QString>
#include <QVector>

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

public:
    enum Role {
        JobIdRole = Qt::UserRole + 1,
        TitleRole,
        UrlRole,
        StatusRole,
        ProgressRole
    };
    Q_ENUM(Role)

    explicit DownloadManager(SettingsStore *settings, ImportManager *importer,
                             DependencyManager *dependencies, BrowserAuthManager *auth,
                             QObject *parent = nullptr);
    ~DownloadManager() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool busy() const { return m_busy; }
    double progress() const { return m_progress; }
    QString statusText() const { return m_statusText; }
    QString lastError() const { return m_lastError; }
    int activeCount() const { return m_activeCount; }

    Q_INVOKABLE void enqueue(const QStringList &urls, const QString &quality, bool authorized = false);
    Q_INVOKABLE void cancelJob(int id);
    Q_INVOKABLE void retryJob(int id);
    Q_INVOKABLE void clearFinished();
    Q_INVOKABLE QString serviceForUrl(const QString &url) const;

signals:
    void busyChanged();
    void progressChanged();
    void statusTextChanged();
    void lastErrorChanged();
    void activeCountChanged();

private:
    struct Job;

    int indexOfJob(int id) const;
    bool isActive(const Job *job) const;
    int concurrencyLimit() const;
    void startQueued();
    void startJob(Job *job);
    void readProcessOutput(Job *job, bool errorOutput);
    void finishCancelled(Job *job);
    void finishFailed(Job *job, const QString &message);
    void finishSucceeded(Job *job);
    bool importDownloadedFiles(Job *job, QString *errorMessage);
    void setJobStatus(Job *job, const QString &status);
    void setJobProgress(Job *job, double progress);
    void removeTempDir(Job *job);
    void updateAggregate();
    void setStatusText(const QString &statusText);
    void setLastError(const QString &lastError);

    SettingsStore *m_settings;
    ImportManager *m_importer;
    DependencyManager *m_dependencies;
    BrowserAuthManager *m_auth;
    LibraryModel *m_library;
    QVector<Job *> m_jobs;
    int m_nextJobId = 1;
    bool m_busy = false;
    double m_progress = 0.0;
    QString m_statusText;
    QString m_lastError;
    int m_activeCount = 0;
};
