#pragma once
#include <QSqlDatabase>
#include <QVariantMap>
#include <QVector>

// Сущность трека, используемая Qt-моделью, импортёрами и переносимым экспортом.
struct Track
{
    int id = 0;
    QString title, artist, album, filePath, sourceUrl, source, format, coverPath;
    QString mediaUrl; // Короткоживущая ссылка для веб-воспроизведения; никогда не сохраняется в SQLite.
    QString coverUrl; // Импортированная обложка кэшируется локально перед сохранением.
    QVariantMap webData;
    qint64 duration = 0, lastPlayed = 0, listenMs = 0;
    int bitrate = 0, rating = 0, playCount = 0, skipCount = 0;
};

// Доступ к базе данных библиотеки (SQLite): треки, плейлисты, история прослушивания и миграция схемы.
class LibraryRepository
{
  public:
    explicit LibraryRepository(const QString& path = {});
    ~LibraryRepository();
    bool ready() const
    {
        return m_ready;
    }
    QString error() const
    {
        return m_error;
    }
    QString path() const
    {
        return m_db.databaseName();
    }
    // Возвращает треки с необязательным поиском, фильтром по плейлисту и сортировкой.
    QVector<Track> tracks(const QString& search = {}, int playlist = 0, const QString& sort = "artist") const;
    Track track(int id) const;
    int count() const;
    bool hasFile(const QString& path) const;
    // Сохраняет трек (или возвращает id существующего), не создавая дубликатов.
    int saveTrack(const Track& track);
    // Привязывает локальный файл к существующему треку (заполняет путь/формат/длительность).
    bool attachFile(int id, const Track& file);
    bool updateMetadata(int id, const QString& title, const QString& artist, const QString& album);
    bool setCover(int id, const QString& path);
    // Перемещает треки между альбомами (например, при переименовании альбома).
    bool setAlbumTracks(const QString& previous, const QString& title, const QVariantList& ids);
    bool setRating(int id, int rating);
    bool removeTrack(int id);
    // Фиксирует факт воспроизведения (счётчик и время последнего запуска).
    bool recordPlay(int id);
    // Записывает событие прослушивания и накапливает статистику (время/пропуски).
    bool recordListen(int id, qint64 ms, bool skipped);
    QVariantList playlists() const;
    int createPlaylist(const QString& name);
    bool renamePlaylist(int id, const QString& name);
    bool deletePlaylist(int id);
    bool addToPlaylist(int playlist, int track);
    bool removeFromPlaylist(int playlist, int track);
    bool transaction();
    bool commit();
    void rollback();

  private:
    bool migrate();
    bool exec(const QString& sql, const QVariantList& args = {}) const;
    int lookup(const QString& table, const QString& name, int artist = 0);
    static Track readTrack(const class QSqlQuery& query);
    QSqlDatabase m_db;
    QString m_connection;
    mutable QString m_error;
    bool m_ready = false;
};
