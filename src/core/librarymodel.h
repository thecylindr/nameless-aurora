#pragma once
// Модель списка треков для QML: фильтрация, сортировка, плейлисты и обложки.

#include <QAbstractListModel>
#include <QVector>
#include <QNetworkAccessManager>
#include <QQueue>
#include <QUrl>
#include <QSet>

#include "data/libraryrepository.h"

class LibraryModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY libraryChanged)
    Q_PROPERTY(int downloadedCount READ downloadedCount NOTIFY libraryChanged)
    Q_PROPERTY(int missingCount READ missingCount NOTIFY libraryChanged)
    Q_PROPERTY(int playlistFilter READ playlistFilter WRITE setPlaylistFilter NOTIFY filterChanged)
    Q_PROPERTY(QString sortOrder READ sortOrder WRITE setSortOrder NOTIFY filterChanged)
    Q_PROPERTY(QVariantList playlists READ playlists NOTIFY libraryChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

  public:
    enum Role
    {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        ArtistRole,
        AlbumRole,
        FilePathRole,
        SourceUrlRole,
        SourceRole,
        FormatRole,
        CoverPathRole,
        DurationRole,
        BitrateRole,
        RatingRole,
        PlayCountRole,
        LastPlayedRole,
        SkipCountRole,
        ListenMsRole,
        AvailableRole
    };
    Q_ENUM(Role)

    explicit LibraryModel(QObject* parent = nullptr, const QString& databasePath = {});
    ~LibraryModel() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const
    {
        return m_tracks.size();
    }
    int totalCount() const
    {
        return m_repository.count();
    }
    int downloadedCount() const
    {
        return m_downloadedCount;
    }
    int missingCount() const
    {
        return qMax(0, totalCount() - m_downloadedCount);
    }
    int playlistFilter() const
    {
        return m_playlistFilter;
    }
    QString sortOrder() const
    {
        return m_sortOrder;
    }
    void setPlaylistFilter(int id);
    void setSortOrder(const QString& order);
    QVariantList playlists() const
    {
        return m_repository.playlists();
    }
    LibraryRepository& repository()
    {
        return m_repository;
    }
    Track trackById(int id) const
    {
        return m_repository.track(id);
    }
    // Список ID треков, у которых на диске есть аудиофайл.
    QVector<int> playableIds() const;
    // Детали треков очереди для отображения.
    Q_INVOKABLE QVariantList queueDetails(const QVariantList& ids) const;
    // Детали одного трека для UI.
    Q_INVOKABLE QVariantMap trackDetails(int id) const;
    // Детали всех треков (например, для экспорта).
    Q_INVOKABLE QVariantList allTrackDetails() const;
    // Обновляет заголовок, исполнителя и альбом трека.
    Q_INVOKABLE bool updateMetadata(int id, const QString& title, const QString& artist, const QString& album);
    // Назначает обложку треку.
    Q_INVOKABLE bool setTrackCover(int id, const QUrl& file);
    // Объединяет треки в альбом и ставит общую обложку.
    Q_INVOKABLE bool setAlbumTracks(const QString& previous, const QString& title, const QVariantList& ids,
                                    const QUrl& cover = {});
    // Ставит в очередь скачивание удалённой обложки.
    void cacheCover(int id, const QUrl& url);
    Q_INVOKABLE void requestEdit(int id)
    {
        emit editRequested(id);
    }
    Q_INVOKABLE void requestAlbumEdit(const QString& title = {})
    {
        emit albumEditRequested(title);
    }
    // Создаёт плейлист и возвращает его ID (0 при ошибке).
    Q_INVOKABLE int createPlaylist(const QString& name);
    Q_INVOKABLE void renamePlaylist(int id, const QString& name);
    Q_INVOKABLE void deletePlaylist(int id);
    Q_INVOKABLE void addToPlaylist(int playlist, int track);
    Q_INVOKABLE void removeFromPlaylist(int playlist, int track);
    // Привязывает аудиофайл к уже существующему треку.
    bool attachFile(int id, const QString& path);
    // Привязывает готовый файл с заранее полученными метаданными (используется при загрузке).
    bool attachPreparedFile(int id, const QString& path, Track metadata);
    // Читает метаданные аудиофайла через ffprobe (статическая, без создания модели).
    static Track probeFile(const QString& filePath, const QString& executable = "ffprobe");
    // Извлекает обложку из файла через ffmpeg.
    static QString extractCover(const QString& filePath, int trackId, const QString& executable = "ffmpeg");
    // Проверяет, есть ли файл уже в медиатеке.
    bool hasFile(const QString& path) const;
    QString searchText() const
    {
        return m_searchText;
    }
    QString lastError() const
    {
        return m_lastError;
    }

    void setSearchText(const QString& text);
    // Добавляет трек из файла, извлекая метаданные и обложку.
    bool addTrack(const QString& filePath, const QString& sourceUrl = {}, const QString& source = {});
    Q_INVOKABLE void setRating(int id, int rating);
    Q_INVOKABLE void removeTrack(int id);
    void incrementPlayCount(int id);
    // Фиксирует факт прослушивания (длительность и признак пропуска).
    void recordListen(int id, qint64 milliseconds, bool skipped);
    // Позиция трека в текущей модели по его ID (-1, если отсутствует).
    int rowForTrackId(int id) const;
    // Подборка рекомендаций по рейтингу, прослушиваниям и «аффинити» исполнителя.
    Q_INVOKABLE QVariantList recommendations(int limit = 12) const;
    // Перечитывает модель из репозитория с текущими фильтром/поиском/сортировкой.
    Q_INVOKABLE void refresh();

  signals:
    void editRequested(int id);
    void albumEditRequested(const QString& title);
    void countChanged();
    void filterChanged();
    void searchTextChanged();
    void lastErrorChanged();
    void libraryChanged();

  private:
    void fetchNextCover();
    void notifyCovers(const QList<int>& ids);
    QString importCover(const QString& file);
    QNetworkAccessManager m_coverNetwork;
    QQueue<QString> m_coverQueue;
    QHash<QString, QList<int>> m_coverTargets;
    int m_coverActive = 0;
    QSet<int> m_changedCovers;
    bool m_coverNotificationPending = false;
    void setError(const QString& error);

    QVector<Track> m_tracks;
    int m_downloadedCount = 0;
    QString m_searchText;
    QString m_lastError;
    LibraryRepository m_repository;
    int m_playlistFilter = 0;
    QString m_sortOrder = "artist";
};
