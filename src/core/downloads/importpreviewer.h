#pragma once

#include <QAbstractListModel>
#include <QByteArray>
#include <QHash>
#include <QVector>

// Предпросмотр ссылок перед импортом: получает метаданные и список треков через yt-dlp,
// позволяет выбрать нужные и передать их в очередь загрузки.
class BrowserAuthManager;
class DependencyManager;
class DownloadManager;
class QProcess;
class SettingsStore;

class ImportPreviewer : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(qint64 totalDuration READ totalDuration NOTIFY totalDurationChanged)
    Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectedCountChanged)
    Q_PROPERTY(qint64 selectedBytes READ selectedBytes NOTIFY selectedBytesChanged)
    Q_PROPERTY(int unknownSizes READ unknownSizes NOTIFY unknownSizesChanged)
    Q_PROPERTY(QString sourceUrl READ sourceUrl NOTIFY sourceUrlChanged)

  public:
    enum Role
    {
        TitleRole = Qt::UserRole + 1,
        ArtistRole,
        UrlRole,
        DurationRole,
        SizeRole,
        SelectedRole
    };
    Q_ENUM(Role)

    explicit ImportPreviewer(SettingsStore* settings, DependencyManager* dependencies, BrowserAuthManager* auth,
                             DownloadManager* downloads, QObject* parent = nullptr);
    ~ImportPreviewer() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool busy() const
    {
        return m_busy;
    }
    QString statusText() const
    {
        return m_statusText;
    }
    QString lastError() const
    {
        return m_lastError;
    }
    qint64 totalDuration() const
    {
        return m_totalDuration;
    }
    int selectedCount() const
    {
        return m_selectedCount;
    }
    qint64 selectedBytes() const
    {
        return m_selectedBytes;
    }
    int unknownSizes() const
    {
        return m_unknownSizes;
    }
    QString sourceUrl() const
    {
        return m_sourceUrl;
    }

    // Разбирает ссылку/плейлист и показывает найденные треки для выбора.
    Q_INVOKABLE void inspect(const QString& url, const QString& quality, bool authorized = false);
    // Поиск на YouTube по текстовому запросу и предпросмотр результатов.
    Q_INVOKABLE void searchYoutube(const QString& query, const QString& quality);
    // Прерывает текущее получение списка.
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void toggle(int index);
    Q_INVOKABLE void selectAll(bool selected);
    // Отправляет выбранные треки в очередь загрузок.
    Q_INVOKABLE void startSelected();

  signals:
    void busyChanged();
    void statusTextChanged();
    void lastErrorChanged();
    void totalDurationChanged();
    void selectedCountChanged();
    void selectedBytesChanged();
    void unknownSizesChanged();
    void sourceUrlChanged();

  private:
    struct Item;

    void startPreview(const QString& url, const QString& quality, bool authorized, bool searching);
    bool m_searching = false;
    QString m_stopReason;
    void parseMetadata(const QByteArray& json);
    void inspectSpotify(const QString& url);
    void updateSummary();
    void finishError(const QString& message);
    void setBusy(bool busy);
    void setStatusText(const QString& statusText);
    void setLastError(const QString& lastError);
    void setSourceUrl(const QString& sourceUrl);

    SettingsStore* m_settings;
    DependencyManager* m_dependencies;
    BrowserAuthManager* m_auth;
    DownloadManager* m_downloads;
    QVector<Item> m_items;
    QProcess* m_process = nullptr;
    QByteArray m_stdout;
    QString m_stderr;
    QString m_quality = QStringLiteral("balanced");
    bool m_authorized = false;
    bool m_busy = false;
    QString m_statusText;
    QString m_lastError;
    qint64 m_totalDuration = 0;
    int m_selectedCount = 0;
    qint64 m_selectedBytes = 0;
    int m_unknownSizes = 0;
    QString m_sourceUrl;
};
