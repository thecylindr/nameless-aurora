#include "libraryrepository.h"
#include "../web/vktext.h"
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>
#include <QSet>
#include <QUuid>

// Реализация LibraryRepository: схема SQLite, миграции, сохранение треков без дубликатов и статистика прослушивания.

LibraryRepository::LibraryRepository(const QString& path)
{
    // Уникальное имя соединения позволяет открывать несколько баз в одном процессе.
    m_connection = "aurora_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connection);
    QString target = path;
    if (target.isEmpty())
        target = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/library.sqlite3";
    QDir().mkpath(QFileInfo(target).absolutePath());
    m_db.setDatabaseName(target);
    if (!m_db.open()) {
        m_error = m_db.lastError().text();
        return;
    }
    // Прагмы включают внешние ключи, таймаут блокировки и журнал WAL для конкурентного доступа.
    m_ready = exec("PRAGMA foreign_keys=ON") && exec("PRAGMA busy_timeout=5000") && exec("PRAGMA journal_mode=WAL") &&
              migrate();
}
LibraryRepository::~LibraryRepository()
{
    m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connection);
}
bool LibraryRepository::exec(const QString& sql, const QVariantList& args) const
{
    QSqlQuery q(m_db);
    if (!q.prepare(sql)) {
        m_error = q.lastError().text();
        return false;
    }
    for (const auto& v : args)
        q.addBindValue(v);
    if (!q.exec()) {
        m_error = q.lastError().text();
        return false;
    }
    return true;
}
bool LibraryRepository::transaction()
{
    if (m_db.transaction())
        return true;
    m_error = m_db.lastError().text();
    return false;
}
bool LibraryRepository::commit()
{
    if (m_db.commit())
        return true;
    m_error = m_db.lastError().text();
    return false;
}
void LibraryRepository::rollback()
{
    m_db.rollback();
}

bool LibraryRepository::migrate()
{
    QSqlQuery version(m_db);
    if (!version.exec("PRAGMA user_version") || !version.next())
        return false;
    const int oldVersion = version.value(0).toInt();
    version.finish();
    if (oldVersion > 2) {
        m_error = "База создана более новой версией Aurora.";
        return false;
    }
    if (oldVersion == 2)
        return true;
    // Наличие таблицы tracks означает старую схему (без user_version).
    const bool legacy = m_db.tables().contains("tracks");
    if (!transaction())
        return false;
    bool ok = true;
    auto run = [&](const QString& sql) {
        if (ok)
            ok = exec(sql);
    };
    if (legacy) {
        // В старых версиях не было номера схемы; добавляем отсутствующие в ранних базах поля.
        const auto record = m_db.record("tracks");
        const QStringList fields{"cover_path", "last_played", "skip_count", "listen_ms"};
        for (const auto& field : fields)
            if (!record.contains(field))
                run("ALTER TABLE tracks ADD COLUMN " + field +
                    (field == "cover_path" ? " TEXT NOT NULL DEFAULT ''" : " INTEGER NOT NULL DEFAULT 0"));
        run("ALTER TABLE tracks RENAME TO tracks_v1");
    }
    const QStringList schema{
        "CREATE TABLE artists(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE)",
        "CREATE TABLE albums(id INTEGER PRIMARY KEY, title TEXT NOT NULL, artist_id INTEGER NOT NULL REFERENCES "
        "artists(id), UNIQUE(title,artist_id))",
        "CREATE TABLE sources(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE)",
        "CREATE TABLE tracks(id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT NOT NULL, artist_id INTEGER NOT NULL "
        "REFERENCES artists(id), album_id INTEGER NOT NULL REFERENCES albums(id), source_id INTEGER NOT NULL "
        "REFERENCES sources(id), file_path TEXT UNIQUE, source_url TEXT NOT NULL DEFAULT '', format TEXT NOT NULL "
        "DEFAULT '', cover_path TEXT NOT NULL DEFAULT '', duration INTEGER NOT NULL DEFAULT 0 CHECK(duration>=0), "
        "bitrate INTEGER NOT NULL DEFAULT 0, rating INTEGER NOT NULL DEFAULT 0 CHECK(rating BETWEEN 0 AND 5), "
        "play_count INTEGER NOT NULL DEFAULT 0, last_played INTEGER NOT NULL DEFAULT 0, skip_count INTEGER NOT NULL "
        "DEFAULT 0, listen_ms INTEGER NOT NULL DEFAULT 0)",
        "CREATE TABLE playlists(id INTEGER PRIMARY KEY, name TEXT NOT NULL CHECK(length(trim(name))>0), created_at "
        "INTEGER NOT NULL)",
        "CREATE TABLE playlist_tracks(playlist_id INTEGER NOT NULL REFERENCES playlists(id) ON DELETE CASCADE, "
        "track_id INTEGER NOT NULL REFERENCES tracks(id) ON DELETE CASCADE, position INTEGER NOT NULL, PRIMARY "
        "KEY(playlist_id,track_id), UNIQUE(playlist_id,position))",
        "CREATE TABLE listening_events(id INTEGER PRIMARY KEY, track_id INTEGER NOT NULL REFERENCES tracks(id) ON "
        "DELETE CASCADE, played_at INTEGER NOT NULL, listened_ms INTEGER NOT NULL CHECK(listened_ms>0), skipped "
        "INTEGER NOT NULL CHECK(skipped IN(0,1)))",
        "CREATE INDEX idx_track_artist ON tracks(artist_id)",
        "CREATE INDEX idx_track_album ON tracks(album_id)",
        "CREATE INDEX idx_track_history ON tracks(last_played DESC)",
        "CREATE INDEX idx_playlist_track ON playlist_tracks(track_id)",
        "CREATE INDEX idx_events_track ON listening_events(track_id,played_at)",
        "CREATE VIEW track_details AS SELECT t.id,t.title,a.name AS artist,b.title AS "
        "album,t.file_path,t.source_url,s.name AS "
        "source,t.format,t.cover_path,t.duration,t.bitrate,t.rating,t.play_count,t.last_played,t.skip_count,t.listen_"
        "ms FROM tracks t JOIN artists a ON a.id=t.artist_id JOIN albums b ON b.id=t.album_id JOIN sources s ON "
        "s.id=t.source_id"};
    for (const auto& sql : schema)
        run(sql);
    if (legacy) {
        run("INSERT INTO artists(name) SELECT DISTINCT artist FROM tracks_v1");
        run("INSERT INTO albums(title,artist_id) SELECT DISTINCT t.album,a.id FROM tracks_v1 t JOIN artists a ON "
            "a.name=t.artist");
        run("INSERT INTO sources(name) SELECT DISTINCT source FROM tracks_v1");
        run("INSERT INTO tracks SELECT "
            "t.id,t.title,a.id,b.id,s.id,NULLIF(t.file_path,''),t.source_url,t.format,t.cover_path,t.duration,t."
            "bitrate,t.rating,t.play_count,t.last_played,t.skip_count,t.listen_ms FROM tracks_v1 t JOIN artists a ON "
            "a.name=t.artist JOIN albums b ON b.title=t.album AND b.artist_id=a.id JOIN sources s ON s.name=t.source");
        run("DROP TABLE tracks_v1");
    }
    run("PRAGMA user_version=2");
    if (!ok || !commit()) {
        rollback();
        return false;
    }
    return true;
}

Track LibraryRepository::readTrack(const QSqlQuery& q)
{
    Track t;
    t.id = q.value(0).toInt();
    t.title = q.value(1).toString();
    t.artist = q.value(2).toString();
    t.album = q.value(3).toString();
    t.filePath = q.value(4).toString();
    t.sourceUrl = q.value(5).toString();
    t.source = q.value(6).toString();
    t.format = q.value(7).toString();
    t.coverPath = q.value(8).toString();
    t.duration = q.value(9).toLongLong();
    t.bitrate = q.value(10).toInt();
    t.rating = q.value(11).toInt();
    t.playCount = q.value(12).toInt();
    t.lastPlayed = q.value(13).toLongLong();
    t.skipCount = q.value(14).toInt();
    t.listenMs = q.value(15).toLongLong();
    return t;
}
QVector<Track> LibraryRepository::tracks(const QString& search, int playlist, const QString& sort) const
{
    QVector<Track> result;
    QSqlQuery q(m_db);
    QString sql = "SELECT d.* FROM track_details d";
    if (playlist > 0)
        sql += " JOIN playlist_tracks p ON p.track_id=d.id WHERE p.playlist_id=?";
    else if (playlist == -1)
        sql += " WHERE d.rating>=4";
    sql += sort == "recent"    ? " ORDER BY d.id DESC"
           : sort == "history" ? " ORDER BY d.last_played DESC"
           : sort == "title"   ? " ORDER BY d.title COLLATE NOCASE"
           : playlist > 0      ? " ORDER BY p.position"
                               : " ORDER BY d.artist COLLATE NOCASE,d.title COLLATE NOCASE";
    q.prepare(sql);
    if (playlist > 0)
        q.addBindValue(playlist);
    if (!q.exec()) {
        m_error = q.lastError().text();
        return result;
    }
    while (q.next()) {
        Track t = readTrack(q);
        // Юникодное сворачивание регистра QString позволяет искать кириллицу в обычном SQLite.
        if (search.isEmpty() || (t.title + "\n" + t.artist + "\n" + t.album).contains(search, Qt::CaseInsensitive))
            result.append(t);
    }
    return result;
}
Track LibraryRepository::track(int id) const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT * FROM track_details WHERE id=?");
    q.addBindValue(id);
    if (q.exec() && q.next())
        return readTrack(q);
    return {};
}
int LibraryRepository::count() const
{
    QSqlQuery q(m_db);
    return q.exec("SELECT COUNT(*) FROM tracks") && q.next() ? q.value(0).toInt() : 0;
}
bool LibraryRepository::hasFile(const QString& path) const
{
    QSqlQuery q(m_db);
    q.prepare("SELECT 1 FROM tracks WHERE file_path=?");
    q.addBindValue(path);
    return q.exec() && q.next();
}
// Находит или создаёт запись в справочнике (artists/albums/sources) и возвращает её id.
int LibraryRepository::lookup(const QString& table, const QString& name, int artist)
{
    const QString column = table == "albums" ? "title" : "name";
    const QString value = name.isNull() ? QStringLiteral("") : name;
    QVariantList args{value};
    if (table == "albums")
        args.append(artist);
    if (!exec("INSERT OR IGNORE INTO " + table + "(" + column + (table == "albums" ? ",artist_id" : "") +
                  ") VALUES (?" + (table == "albums" ? ",?" : "") + ")",
              args))
        return 0;
    QSqlQuery q(m_db);
    q.prepare("SELECT id FROM " + table + " WHERE " + column + "=?" + (table == "albums" ? " AND artist_id=?" : ""));
    for (const auto& v : args)
        q.addBindValue(v);
    return q.exec() && q.next() ? q.value(0).toInt() : 0;
}
int LibraryRepository::saveTrack(const Track& t)
{
    m_error.clear();
    if (t.title.trimmed().isEmpty()) {
        m_error = "Название трека пустое.";
        return 0;
    }
    QSqlQuery existing(m_db);
    if (!t.filePath.isEmpty()) {
        existing.prepare("SELECT id FROM tracks WHERE file_path=?");
        existing.addBindValue(t.filePath);
        if (existing.exec() && existing.next())
            return existing.value(0).toInt();
    }
    // Правки метаданных пользователем не должны создавать вторую запись при повторном импорте.
    if (t.filePath.isEmpty() && !t.sourceUrl.isEmpty()) {
        existing.prepare("SELECT id FROM tracks WHERE source_url=? ORDER BY file_path IS NULL LIMIT 1");
        existing.addBindValue(t.sourceUrl);
        if (existing.exec() && existing.next()) {
            const int id = existing.value(0).toInt();
            existing.finish();
            const auto previous = track(id);
            if (t.source == "VK") {
                // Для VK исправляем «сломанную» кодировку, но не затираем уже корректные поля.
                const auto fixed = [](const QString& before, const QString& after) {
                    return VkText::broken(before) && !after.isEmpty() && !VkText::broken(after) ? after : before;
                };
                const auto title = fixed(previous.title, t.title), artist = fixed(previous.artist, t.artist),
                           album = fixed(previous.album, t.album);
                if ((title != previous.title || artist != previous.artist || album != previous.album) &&
                    !updateMetadata(id, title, artist, album))
                    return 0;
            }
            return id;
        }
    }
    // Если пришёл локальный файл с известной веб-ссылкой, привязываем его к существующему онлайн-треку.
    if (!t.filePath.isEmpty() && !t.sourceUrl.isEmpty()) {
        existing.prepare("SELECT id FROM tracks WHERE source_url=? AND file_path IS NULL LIMIT 1");
        existing.addBindValue(t.sourceUrl);
        if (existing.exec() && existing.next()) {
            const int id = existing.value(0).toInt();
            existing.finish();
            return attachFile(id, t) ? id : 0;
        }
    }
    const int artist = lookup("artists", t.artist.trimmed().isEmpty() ? QStringLiteral("Неизвестный исполнитель")
                                                                      : t.artist.trimmed());
    const int album = lookup("albums", t.album, artist);
    const int source = lookup("sources", t.source.isEmpty() ? QStringLiteral("Local") : t.source);
    if (!artist || !album || !source)
        return 0;
    // Ищем дубликат по названию/исполнителю, чтобы не плодить записи при повторном импорте.
    if (t.filePath.isEmpty()) {
        existing.prepare("SELECT id FROM tracks WHERE title=? AND artist_id=? AND (file_path IS NOT NULL OR "
                         "(source_id=? AND source_url=?)) ORDER BY file_path IS NULL LIMIT 1");
        existing.addBindValue(t.title);
        existing.addBindValue(artist);
        existing.addBindValue(source);
        existing.addBindValue(t.sourceUrl.isNull() ? QStringLiteral("") : t.sourceUrl);
        if (existing.exec() && existing.next())
            return existing.value(0).toInt();
    } else {
        existing.prepare("SELECT id FROM tracks WHERE title=? AND artist_id=? AND file_path IS NULL LIMIT 1");
        existing.addBindValue(t.title);
        existing.addBindValue(artist);
        if (existing.exec() && existing.next()) {
            int id = existing.value(0).toInt();
            return attachFile(id, t) ? id : 0;
        }
    }
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO "
              "tracks(title,artist_id,album_id,source_id,file_path,source_url,format,cover_path,duration,bitrate,"
              "rating) VALUES(?,?,?,?,?,?,?,?,?,?,?)");
    q.addBindValue(t.title);
    q.addBindValue(artist);
    q.addBindValue(album);
    q.addBindValue(source);
    q.addBindValue(t.filePath.isEmpty() ? QVariant() : QVariant(t.filePath));
    for (const QString& v : {t.sourceUrl, t.format, t.coverPath})
        q.addBindValue(v.isNull() ? QStringLiteral("") : v);
    q.addBindValue(qMax<qint64>(0, t.duration));
    q.addBindValue(t.bitrate);
    q.addBindValue(qBound(0, t.rating, 5));
    if (!q.exec()) {
        m_error = q.lastError().text();
        return 0;
    }
    return q.lastInsertId().toInt();
}
bool LibraryRepository::attachFile(int id, const Track& t)
{
    return exec("UPDATE tracks SET file_path=?,format=?,duration=?,bitrate=?,cover_path=CASE WHEN ?<>'' THEN ? ELSE "
                "cover_path END WHERE id=?",
                {t.filePath, t.format, qMax<qint64>(0, t.duration), t.bitrate,
                 t.coverPath.isNull() ? QStringLiteral("") : t.coverPath,
                 t.coverPath.isNull() ? QStringLiteral("") : t.coverPath, id});
}
bool LibraryRepository::updateMetadata(int id, const QString& title, const QString& artist, const QString& album)
{
    if (title.trimmed().isEmpty() || !track(id).id) {
        m_error = "Укажи название существующего трека.";
        return false;
    }
    const int artistId =
        lookup("artists", artist.trimmed().isEmpty() ? QStringLiteral("Неизвестный исполнитель") : artist.trimmed());
    const int albumId = artistId ? lookup("albums", album.trimmed(), artistId) : 0;
    return albumId && exec("UPDATE tracks SET title=?,artist_id=?,album_id=? WHERE id=?",
                           {title.trimmed(), artistId, albumId, id});
}
bool LibraryRepository::setCover(int id, const QString& path)
{
    return exec("UPDATE tracks SET cover_path=? WHERE id=?", {path, id});
}
bool LibraryRepository::setAlbumTracks(const QString& previous, const QString& title, const QVariantList& ids)
{
    if (title.trimmed().isEmpty()) {
        m_error = "Укажи название альбома.";
        return false;
    }
    QSet<int> selected;
    for (const auto& id : ids) {
        if (!track(id.toInt()).id) {
            m_error = "Трек больше не существует.";
            return false;
        }
        selected.insert(id.toInt());
    }
    if (!transaction())
        return false;
    // Выбранным трекам присваиваем новый альбом, остальным из прежнего альбома — очищаем поле.
    for (const auto& t : tracks()) {
        if (!selected.contains(t.id) && (previous.isEmpty() || t.album != previous))
            continue;
        const QString album = selected.contains(t.id) ? title.trimmed() : QString();
        if (!updateMetadata(t.id, t.title, t.artist, album)) {
            rollback();
            return false;
        }
    }
    if (!commit()) {
        rollback();
        return false;
    }
    return true;
}
bool LibraryRepository::setRating(int id, int rating)
{
    return exec("UPDATE tracks SET rating=? WHERE id=?", {qBound(0, rating, 5), id});
}
bool LibraryRepository::removeTrack(int id)
{
    return exec("DELETE FROM tracks WHERE id=?", {id});
}
bool LibraryRepository::recordPlay(int id)
{
    return exec("UPDATE tracks SET play_count=play_count+1,last_played=? WHERE id=?",
                {QDateTime::currentMSecsSinceEpoch(), id});
}
bool LibraryRepository::recordListen(int id, qint64 ms, bool skipped)
{
    if (ms <= 0)
        return true;
    if (!transaction())
        return false;
    // Событие прослушивания и агрегированная статистика пишутся атомарно в одной транзакции.
    if (!exec("INSERT INTO listening_events(track_id,played_at,listened_ms,skipped) VALUES(?,?,?,?)",
              {id, QDateTime::currentMSecsSinceEpoch(), ms, skipped ? 1 : 0}) ||
        !exec("UPDATE tracks SET listen_ms=listen_ms+?,skip_count=skip_count+? WHERE id=?",
              {ms, skipped ? 1 : 0, id}) ||
        !commit()) {
        rollback();
        return false;
    }
    return true;
}
QVariantList LibraryRepository::playlists() const
{
    QVariantList result;
    QSqlQuery q(m_db);
    // Для каждого плейлиста возвращаем имя, число треков и сколько из них доступны локально.
    q.exec("SELECT p.id,p.name,COUNT(pt.track_id),SUM(CASE WHEN t.file_path IS NOT NULL THEN 1 ELSE 0 END) FROM "
           "playlists p LEFT JOIN playlist_tracks pt ON pt.playlist_id=p.id LEFT JOIN tracks t ON t.id=pt.track_id "
           "GROUP BY p.id ORDER BY p.created_at,p.id");
    while (q.next())
        result.append(
            QVariantMap{{"id", q.value(0)}, {"name", q.value(1)}, {"count", q.value(2)}, {"available", q.value(3)}});
    return result;
}
int LibraryRepository::createPlaylist(const QString& name)
{
    if (name.trimmed().isEmpty()) {
        m_error = "Введите название плейлиста.";
        return 0;
    }
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO playlists(name,created_at) VALUES(?,?)");
    q.addBindValue(name.trimmed());
    q.addBindValue(QDateTime::currentMSecsSinceEpoch());
    if (!q.exec()) {
        m_error = q.lastError().text();
        return 0;
    }
    return q.lastInsertId().toInt();
}
bool LibraryRepository::renamePlaylist(int id, const QString& name)
{
    return exec("UPDATE playlists SET name=? WHERE id=?", {name.trimmed(), id});
}
bool LibraryRepository::deletePlaylist(int id)
{
    return exec("DELETE FROM playlists WHERE id=?", {id});
}
bool LibraryRepository::addToPlaylist(int playlist, int track)
{
    return exec("INSERT OR IGNORE INTO playlist_tracks(playlist_id,track_id,position) SELECT "
                "?,?,COALESCE(MAX(position)+1,0) FROM playlist_tracks WHERE playlist_id=?",
                {playlist, track, playlist});
}
bool LibraryRepository::removeFromPlaylist(int playlist, int track)
{
    return exec("DELETE FROM playlist_tracks WHERE playlist_id=? AND track_id=?", {playlist, track});
}
