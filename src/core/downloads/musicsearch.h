#pragma once
#include <QObject>
#include <QVariantList>
#include <QVariantMap>
#include <QHash>
#include <QSet>
#include <QTimer>
class AudioSources;
class LibraryModel;
class DownloadManager;
class DependencyManager;
class WebLoginManager;

// Раздел «Музыка»: поиск и подбор новинок по источникам с отдельными воркерами,
// не связанными с операциями автоподбора недостающих треков.
class MusicSearch : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList results READ results NOTIFY changed)
    Q_PROPERTY(QVariantMap sourceStates READ sourceStates NOTIFY changed)
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool resolving READ resolving NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
    Q_PROPERTY(QString query READ query NOTIFY changed)
    Q_PROPERTY(QString auditionKey READ auditionKey NOTIFY changed)
    Q_PROPERTY(QStringList enabledSources READ enabledSources WRITE setEnabledSources NOTIFY preferencesChanged)
    Q_PROPERTY(QString taste READ taste WRITE setTaste NOTIFY preferencesChanged)
  public:
    MusicSearch(LibraryModel*, DownloadManager*, DependencyManager*, WebLoginManager*, QObject* parent = nullptr);
    QVariantList results() const
    {
        return m_results;
    }
    QVariantMap sourceStates() const
    {
        return m_states;
    }
    bool busy() const
    {
        return !m_pending.isEmpty();
    }
    bool resolving() const
    {
        return !m_downloadKey.isEmpty();
    }
    QString status() const
    {
        return m_status;
    }
    QString query() const
    {
        return m_query;
    }
    QString auditionKey() const
    {
        return m_auditionKey;
    }
    QStringList enabledSources() const
    {
        return m_sources;
    }
    QString taste() const
    {
        return m_taste;
    }
    void setEnabledSources(const QStringList& sources);
    void setTaste(const QString& taste);
    // Поиск по текстовому запросу во всех включённых источниках.
    Q_INVOKABLE void search(const QString& query);
    // Подбор рекомендаций на основе вкуса пользователя.
    Q_INVOKABLE void discover();
    Q_INVOKABLE void cancel();
    // Скачивает выбранную запись (VK — напрямую, остальные — через воркер источника).
    Q_INVOKABLE void download(const QString& key);
    // Запускает онлайн-прослушивание выбранной записи.
    Q_INVOKABLE void listen(const QString& key);
    void reportPlaybackError(const QString& key, const QString& error);
    // Останавливает текущее прослушивание.
    Q_INVOKABLE void cancelAudition();
    // Открывает запись в её сервисе (браузером).
    Q_INVOKABLE void openInService(const QString& key);
    Q_INVOKABLE int libraryId(const QString& key) const
    {
        return m_saved.value(key);
    }
  signals:
    void changed();
    void preferencesChanged();
    void playRequested(const QVariantMap& candidate);
    void playLibraryRequested(int id);

  private:
    void begin(const QString& query, bool recommendations);
    void receive(const QString& source, const QVariantList& results, const QString& status);
    void rebuild();
    LibraryModel* m_library;
    DownloadManager* m_downloads;
    WebLoginManager* m_web;
    QHash<QString, AudioSources*> m_workers;
    QHash<QString, QVariantList> m_batches;
    QHash<QString, int> m_saved;
    QSet<QString> m_pending;
    QVariantList m_results;
    QVariantMap m_states;
    QStringList m_sources;
    QString m_taste, m_query, m_status, m_downloadKey, m_downloadSource;
    AudioSources* m_audition = nullptr;
    QString m_auditionKey;
    QVariantMap m_auditionCandidate;
    int m_auditionRequest = 0, m_auditionSerial = 0;
    QTimer m_auditionTimer;
};
