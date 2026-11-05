#pragma once
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QElapsedTimer>
#include <QVariantMap>
#include <QVariantList>
#include <functional>
#include <QQueue>
#include <QSet>

// Поиск музыки по источникам (YouTube, SoundCloud, Zaycev.net, Hitmo, Internet Archive)
// и автоподбор недостающих треков библиотеки с учётом порядка источников.
class LibraryModel;
class DownloadManager;
class DependencyManager;
class BrowserSession;
class OpenCatalog;
class ZaycevCatalog;

class AudioSources : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap target READ target NOTIFY targetChanged)
    Q_PROPERTY(QVariantList results READ results NOTIFY resultsChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
    Q_PROPERTY(QString source READ source NOTIFY changed)
    Q_PROPERTY(QStringList sources READ sources CONSTANT)
    Q_PROPERTY(bool recovering READ recovering NOTIFY changed)
    Q_PROPERTY(QString recoveryStatus READ recoveryStatus NOTIFY changed)
    Q_PROPERTY(int recoveryPending READ recoveryPending NOTIFY changed)
    Q_PROPERTY(bool automaticRecovery READ automaticRecovery WRITE setAutomaticRecovery NOTIFY changed)
    Q_PROPERTY(bool extendedRecovery READ extendedRecovery WRITE setExtendedRecovery NOTIFY changed)
    Q_PROPERTY(QStringList recoverySources READ recoverySources WRITE setRecoverySources NOTIFY changed)
  public:
    AudioSources(LibraryModel* library, DownloadManager* downloads, DependencyManager* tools, QObject* parent = nullptr,
                 std::function<BrowserSession*(QObject*)> browserFactory = {}, bool internalWorker = false);
    ~AudioSources() override;
    QVariantMap target() const
    {
        return m_target;
    }
    QVariantList results() const
    {
        return m_results;
    }
    bool busy() const
    {
        return m_busy;
    }
    QString status() const
    {
        return m_status;
    }
    QString source() const
    {
        return m_source;
    }
    QStringList sources() const
    {
        return {"YouTube", "SoundCloud", "Internet Archive", "Zaycev.net", "Hitmo"};
    }
    bool recovering() const
    {
        return m_recoveryId > 0 || !m_recoveryQueue.isEmpty();
    }
    QString recoveryStatus() const
    {
        return m_recoveryStatus;
    }
    int recoveryPending() const
    {
        return m_recoveryQueue.size() + (m_recoveryId > 0 ? 1 : 0);
    }
    bool automaticRecovery() const
    {
        return m_automaticRecovery;
    }
    void setAutomaticRecovery(bool value);
    bool extendedRecovery() const
    {
        return m_extendedRecovery;
    }
    void setExtendedRecovery(bool value);
    QStringList recoverySources() const
    {
        return m_recoverySources;
    }
    void setRecoverySources(const QStringList& sources);
    // Открывает диалог подбора для трека библиотеки и запускает поиск по источникам.
    Q_INVOKABLE void openTrack(int id);
    // Поиск в режиме каталога (для раздела «Музыка»), без привязки к конкретному треку.
    void searchCatalog(const QString& source, const QString& query);
    // Сразу ставит найденного кандидата в очередь загрузки.
    void downloadCandidate(const QVariantMap& candidate);
    // Разрешает поток аудио для прослушивания без скачивания.
    void previewCandidate(const QVariantMap& candidate);
    // Поиск в одном указанном источнике.
    Q_INVOKABLE void search(const QString& source, const QString& query = {});
    // Поиск сразу во всех включённых источниках (параллельно).
    Q_INVOKABLE void searchAll(const QString& query = {});
    // Выбирает вариант из результатов и запускает получение аудио.
    Q_INVOKABLE void choose(int index);
    // Скачивание по прямой ссылке на запись поддерживаемого сервиса.
    Q_INVOKABLE void downloadLink(const QString& url);
    // Отменяет текущую операцию.
    Q_INVOKABLE void cancel();
    // Закрывает диалог и при необходимости продолжает автоподбор.
    Q_INVOKABLE void close();
    // Добавляет трек в очередь автоподбора недостающего файла.
    Q_INVOKABLE void recoverTrack(int id);
    // Автоподбор всех отсутствующих файлов текущей выборки библиотеки.
    Q_INVOKABLE void recoverMissing();
    // Полностью останавливает автоподбор.
    Q_INVOKABLE void cancelRecovery();
    // Оценка похожести кандидата на целевой трек (по названию, исполнителю и длительности).
    static int matchScore(const QVariantMap& target, const QVariantMap& candidate);
  signals:
    void opened();
    void targetChanged();
    void resultsChanged();
    void changed();
    void queued();
    void catalogQueued(int trackId);
    void operationFinished();
    void previewResolved(const QVariantMap& candidate, const QString& error);

  private:
    void stopWork(bool keepBrowser = false);
    void finish(const QString& status);
    void tick();
    void queue(const QVariantMap& candidate);
    void acceptResults(const QVariantList& candidates);
    void searchPublicBrowser();
    void pumpSearches();
    void receiveSearch(const QString& source);
    void ensureZaycev();
    void resolveStream();
    void startRecovery();
    void recoveryStep();
    void nextRecoverySource();
    void finishRecoveryTrack();
    QString script(const QString& name) const;
    LibraryModel* m_library;
    DownloadManager* m_downloads;
    DependencyManager* m_tools;
    BrowserSession* m_browser = nullptr;
    OpenCatalog* m_catalog = nullptr;
    ZaycevCatalog* m_zaycev = nullptr;
    QHash<QString, QPair<qint64, QVariantList>> m_searchCache;
    std::function<BrowserSession*(QObject*)> m_browserFactory;
    QProcess* m_process = nullptr;
    QTimer m_timer;
    QElapsedTimer m_elapsed;
    QVariantMap m_target, m_candidate;
    QVariantList m_results;
    QString m_source, m_status, m_query, m_searchError;
    QByteArray m_output;
    int m_generation = 0, m_scriptId = 0;
    bool m_busy = false, m_resolving = false, m_ready = false, m_pending = false, m_submitted = false;
    bool m_waitingTools = false, m_internalWorker = false, m_automaticRecovery = true, m_dialogOpen = false;
    bool m_extendedRecovery = false;
    bool m_catalogMode = false;
    bool m_preview = false;
    QHash<QString, int> m_sourceFailures;
    QHash<QString, qint64> m_sourceCooldown;
    QStringList m_recoverySources;
    AudioSources* m_worker = nullptr;
    QHash<QString, AudioSources*> m_searchWorkers;
    QQueue<QString> m_searchQueue;
    QSet<QString> m_searchPending;
    QStringList m_searchFailures;
    bool m_searchAll = false;
    QHash<QString, int> m_recoverySourceAttempts;
    QQueue<int> m_recoveryQueue;
    QSet<int> m_recoveryIds;
    int m_recoveryId = 0, m_recoverySource = 0;
    int m_recoveryCandidate = -1, m_recoveryAttempts = 0;
    enum RecoveryPhase
    {
        Idle,
        Searching,
        Choosing,
        Downloading
    };
    RecoveryPhase m_recoveryPhase = Idle;
    QString m_recoveryStatus;
};
