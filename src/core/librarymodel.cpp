#include "librarymodel.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>
#include <QtMath>
#include <algorithm>

LibraryModel::LibraryModel(QObject *parent) : QAbstractListModel(parent) {
    m_connectionName = QStringLiteral("aurora_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    db.setDatabaseName(QDir(dataDir).filePath(QStringLiteral("library.sqlite3")));
    if (!db.open()) {
        setError(db.lastError().text());
        return;
    }
    if (createSchema())
        refresh();
}

LibraryModel::~LibraryModel() {
    QSqlDatabase::removeDatabase(m_connectionName);
}

int LibraryModel::rowCount(const QModelIndex &parent) const {
    return parent.isValid() ? 0 : m_tracks.size();
}

QVariant LibraryModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tracks.size())
        return {};
    const Track &track = m_tracks.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case TitleRole: return track.title;
    case IdRole: return track.id;
    case ArtistRole: return track.artist;
    case AlbumRole: return track.album;
    case FilePathRole: return track.filePath;
    case SourceUrlRole: return track.sourceUrl;
    case SourceRole: return track.source;
    case FormatRole: return track.format;
    case CoverPathRole: return track.coverPath;
    case DurationRole: return track.duration;
    case BitrateRole: return track.bitrate;
    case RatingRole: return track.rating;
    case PlayCountRole: return track.playCount;
    case LastPlayedRole: return track.lastPlayed;
    case SkipCountRole: return track.skipCount;
    case ListenMsRole: return track.listenMs;
    default: return {};
    }
}

QHash<int, QByteArray> LibraryModel::roleNames() const {
    return {
        {IdRole, "trackId"}, {TitleRole, "title"}, {ArtistRole, "artist"},
        {AlbumRole, "album"}, {FilePathRole, "filePath"}, {SourceUrlRole, "sourceUrl"},
        {SourceRole, "source"}, {FormatRole, "format"}, {CoverPathRole, "coverPath"},
        {DurationRole, "duration"}, {BitrateRole, "bitrate"}, {RatingRole, "rating"},
        {PlayCountRole, "playCount"}, {LastPlayedRole, "lastPlayed"},
        {SkipCountRole, "skipCount"}, {ListenMsRole, "listenMs"}
    };
}

void LibraryModel::setSearchText(const QString &text) {
    if (text == m_searchText)
        return;
    m_searchText = text;
    emit searchTextChanged();
    refresh();
}

bool LibraryModel::createSchema() {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    bool ok = query.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS tracks ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "title TEXT NOT NULL,"
        "artist TEXT NOT NULL DEFAULT '',"
        "album TEXT NOT NULL DEFAULT '',"
        "file_path TEXT NOT NULL UNIQUE,"
        "source_url TEXT NOT NULL DEFAULT '',"
        "source TEXT NOT NULL DEFAULT 'Local',"
        "format TEXT NOT NULL DEFAULT '',"
        "cover_path TEXT NOT NULL DEFAULT '',"
        "duration INTEGER NOT NULL DEFAULT 0,"
        "bitrate INTEGER NOT NULL DEFAULT 0,"
        "rating INTEGER NOT NULL DEFAULT 0,"
        "play_count INTEGER NOT NULL DEFAULT 0,"
        "last_played INTEGER NOT NULL DEFAULT 0,"
        "skip_count INTEGER NOT NULL DEFAULT 0,"
        "listen_ms INTEGER NOT NULL DEFAULT 0)"));
    QSet<QString> columns;
    if (query.exec(QStringLiteral("PRAGMA table_info(tracks)"))) {
        while (query.next())
            columns.insert(query.value(1).toString());
    }
    const QList<QPair<QString, QString>> migrations = {
        {QStringLiteral("cover_path"), QStringLiteral("TEXT NOT NULL DEFAULT ''")},
        {QStringLiteral("last_played"), QStringLiteral("INTEGER NOT NULL DEFAULT 0")},
        {QStringLiteral("skip_count"), QStringLiteral("INTEGER NOT NULL DEFAULT 0")},
        {QStringLiteral("listen_ms"), QStringLiteral("INTEGER NOT NULL DEFAULT 0")}
    };
    for (const auto &migration : migrations) {
        if (!columns.contains(migration.first))
            ok = query.exec(QStringLiteral("ALTER TABLE tracks ADD COLUMN %1 %2")
                                .arg(migration.first, migration.second)) && ok;
    }
    query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_tracks_artist ON tracks(artist)"));
    query.exec(QStringLiteral("CREATE INDEX IF NOT EXISTS idx_tracks_history ON tracks(last_played, play_count)"));
    if (!ok)
        setError(query.lastError().text());
    return ok;
}

Track LibraryModel::probeFile(const QString &filePath) const {
    Track track;
    const QFileInfo info(filePath);
    track.filePath = info.absoluteFilePath();
    track.title = info.completeBaseName();
    track.artist = QStringLiteral("Неизвестный исполнитель");
    track.format = info.suffix().toUpper();

    QProcess ffprobe;
    ffprobe.start(QStringLiteral("ffprobe"), {
        QStringLiteral("-v"), QStringLiteral("quiet"),
        QStringLiteral("-print_format"), QStringLiteral("json"),
        QStringLiteral("-show_format"), QStringLiteral("-show_streams"), filePath
    });
    if (!ffprobe.waitForFinished(8000) || ffprobe.exitCode() != 0)
        return track;

    const QJsonObject root = QJsonDocument::fromJson(ffprobe.readAllStandardOutput()).object();
    const QJsonObject format = root.value(QStringLiteral("format")).toObject();
    const QJsonObject tags = format.value(QStringLiteral("tags")).toObject();
    track.title = tags.value(QStringLiteral("title")).toString(track.title);
    track.artist = tags.value(QStringLiteral("artist")).toString(
        tags.value(QStringLiteral("ARTIST")).toString(track.artist));
    track.album = tags.value(QStringLiteral("album")).toString(
        tags.value(QStringLiteral("ALBUM")).toString());
    track.duration = qRound64(format.value(QStringLiteral("duration")).toString().toDouble() * 1000.0);
    track.bitrate = format.value(QStringLiteral("bit_rate")).toString().toInt() / 1000;
    if (track.bitrate == 0)
        track.bitrate = static_cast<int>(info.size() * 8 / qMax<qint64>(1, track.duration));
    return track;
}

bool LibraryModel::addTrack(const QString &filePath, const QString &sourceUrl, const QString &source) {
    const QFileInfo info(filePath);
    if (!info.exists() || !info.isFile())
        return false;
    Track track = probeFile(info.absoluteFilePath());
    track.sourceUrl = sourceUrl;
    track.source = source.isEmpty() ? QStringLiteral("Local") : source;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery duplicate(db);
    duplicate.prepare(QStringLiteral("SELECT 1 FROM tracks WHERE file_path = ?"));
    duplicate.addBindValue(track.filePath);
    if (duplicate.exec() && duplicate.next())
        return false;

    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT INTO tracks (title, artist, album, file_path, source_url, source, format, duration, bitrate) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    query.addBindValue(track.title);
    query.addBindValue(track.artist);
    query.addBindValue(track.album.isNull() ? QStringLiteral("") : track.album);
    query.addBindValue(track.filePath);
    query.addBindValue(track.sourceUrl.isNull() ? QStringLiteral("") : track.sourceUrl);
    query.addBindValue(track.source);
    query.addBindValue(track.format);
    query.addBindValue(track.duration);
    query.addBindValue(track.bitrate);
    if (!query.exec()) {
        if (query.lastError().nativeErrorCode() != QStringLiteral("19"))
            setError(query.lastError().text());
        return false;
    }
    const int trackId = query.lastInsertId().toInt();
    const QString coverPath = extractCover(track.filePath, trackId);
    if (!coverPath.isEmpty()) {
        QSqlQuery cover(db);
        cover.prepare(QStringLiteral("UPDATE tracks SET cover_path = ? WHERE id = ?"));
        cover.addBindValue(coverPath);
        cover.addBindValue(trackId);
        cover.exec();
    }
    refresh();
    return true;
}

QString LibraryModel::extractCover(const QString &filePath, int trackId) const {
    const QString coverDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                             + QStringLiteral("/covers");
    QDir().mkpath(coverDir);
    const QString coverPath = QDir(coverDir).filePath(QStringLiteral("%1.jpg").arg(trackId));
    QProcess ffmpeg;
    ffmpeg.start(QStringLiteral("ffmpeg"), {
        QStringLiteral("-hide_banner"), QStringLiteral("-loglevel"), QStringLiteral("error"),
        QStringLiteral("-y"), QStringLiteral("-i"), filePath,
        QStringLiteral("-map"), QStringLiteral("0:v:0"), QStringLiteral("-frames:v"), QStringLiteral("1"),
        QStringLiteral("-vf"), QStringLiteral("scale=640:640:force_original_aspect_ratio_decrease"), coverPath
    });
    if (!ffmpeg.waitForFinished(5000) || ffmpeg.exitCode() != 0 || !QFileInfo::exists(coverPath)) {
        QFile::remove(coverPath);
        return {};
    }
    return coverPath;
}

void LibraryModel::setRating(int id, int rating) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    query.prepare(QStringLiteral("UPDATE tracks SET rating = ? WHERE id = ?"));
    query.addBindValue(qBound(0, rating, 5));
    query.addBindValue(id);
    if (!query.exec())
        setError(query.lastError().text());
    refresh();
}

void LibraryModel::removeTrack(int id) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    query.prepare(QStringLiteral("DELETE FROM tracks WHERE id = ?"));
    query.addBindValue(id);
    if (!query.exec())
        setError(query.lastError().text());
    refresh();
}

void LibraryModel::incrementPlayCount(int id) {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    query.prepare(QStringLiteral("UPDATE tracks SET play_count = play_count + 1, last_played = ? WHERE id = ?"));
    query.addBindValue(QDateTime::currentMSecsSinceEpoch());
    query.addBindValue(id);
    query.exec();
    refresh();
}

void LibraryModel::recordListen(int id, qint64 milliseconds, bool skipped) {
    if (id <= 0 || milliseconds <= 0)
        return;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    query.prepare(QStringLiteral("UPDATE tracks SET listen_ms = listen_ms + ?, "
                                 "skip_count = skip_count + ? WHERE id = ?"));
    query.addBindValue(milliseconds);
    query.addBindValue(skipped ? 1 : 0);
    query.addBindValue(id);
    query.exec();
    emit libraryChanged();
}

int LibraryModel::rowForTrackId(int id) const {
    for (int row = 0; row < m_tracks.size(); ++row) {
        if (m_tracks.at(row).id == id)
            return row;
    }
    return -1;
}

QVariantList LibraryModel::recommendations(int limit) const {
    QVariantList result;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);
    if (!query.exec(QStringLiteral("SELECT id, title, artist, album, file_path, source_url, source, format, "
                                   "cover_path, duration, bitrate, rating, play_count, last_played, "
                                   "skip_count, listen_ms FROM tracks")))
        return result;

    QVector<Track> tracks;
    QHash<QString, double> artistAffinity;
    while (query.next()) {
        Track track;
        track.id = query.value(0).toInt();
        track.title = query.value(1).toString();
        track.artist = query.value(2).toString();
        track.album = query.value(3).toString();
        track.filePath = query.value(4).toString();
        track.sourceUrl = query.value(5).toString();
        track.source = query.value(6).toString();
        track.format = query.value(7).toString();
        track.coverPath = query.value(8).toString();
        track.duration = query.value(9).toLongLong();
        track.bitrate = query.value(10).toInt();
        track.rating = query.value(11).toInt();
        track.playCount = query.value(12).toInt();
        track.lastPlayed = query.value(13).toLongLong();
        track.skipCount = query.value(14).toInt();
        track.listenMs = query.value(15).toLongLong();
        artistAffinity[track.artist] += track.rating * 3.0 + track.playCount * 1.5
                                        + track.listenMs / 3600000.0 - track.skipCount * 0.8;
        tracks.append(track);
    }

    struct ScoredTrack { Track track; double score; };
    QVector<ScoredTrack> scored;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (const Track &track : tracks) {
        const double hours = track.listenMs / 3600000.0;
        const double daysAgo = track.lastPlayed > 0 ? (now - track.lastPlayed) / 86400000.0 : 3650.0;
        double score = track.rating * 12.0 + qLn(1.0 + track.playCount) * 5.0 + hours * 2.0
                       - track.skipCount * 3.0 + artistAffinity.value(track.artist) * 0.45;
        if (track.lastPlayed > 0)
            score += 8.0 * qExp(-daysAgo / 21.0);
        if (track.playCount == 0 && artistAffinity.value(track.artist) > 0)
            score += 9.0;
        scored.append({track, score});
    }
    std::stable_sort(scored.begin(), scored.end(), [](const ScoredTrack &a, const ScoredTrack &b) {
        if (!qFuzzyCompare(a.score, b.score))
            return a.score > b.score;
        return a.track.title.compare(b.track.title, Qt::CaseInsensitive) < 0;
    });

    QHash<QString, int> artistCounts;
    const int maximum = qBound(1, limit, 50);
    for (const ScoredTrack &item : scored) {
        if (artistCounts.value(item.track.artist) >= 2)
            continue;
        const Track &track = item.track;
        result.append(QVariantMap {
            {QStringLiteral("trackId"), track.id}, {QStringLiteral("title"), track.title},
            {QStringLiteral("artist"), track.artist}, {QStringLiteral("album"), track.album},
            {QStringLiteral("filePath"), track.filePath}, {QStringLiteral("source"), track.source},
            {QStringLiteral("format"), track.format}, {QStringLiteral("coverPath"), track.coverPath},
            {QStringLiteral("duration"), track.duration}, {QStringLiteral("rating"), track.rating},
            {QStringLiteral("playCount"), track.playCount}, {QStringLiteral("reason"),
                track.playCount == 0 ? QStringLiteral("Новое от знакомого исполнителя")
                                     : QStringLiteral("По вашим прослушиваниям")}
        });
        ++artistCounts[track.artist];
        if (result.size() == maximum)
            break;
    }
    return result;
}

void LibraryModel::refresh() {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isOpen())
        return;
    QSqlQuery query(db);
    QString sql = QStringLiteral("SELECT id, title, artist, album, file_path, source_url, source, format, "
                                 "cover_path, duration, bitrate, rating, play_count, last_played, "
                                 "skip_count, listen_ms FROM tracks");
    if (!m_searchText.isEmpty())
        sql += QStringLiteral(" WHERE title LIKE ? OR artist LIKE ? OR album LIKE ?");
    sql += QStringLiteral(" ORDER BY artist COLLATE NOCASE, title COLLATE NOCASE");
    query.prepare(sql);
    if (!m_searchText.isEmpty()) {
        const QString pattern = QStringLiteral("%") + m_searchText + QStringLiteral("%");
        query.addBindValue(pattern);
        query.addBindValue(pattern);
        query.addBindValue(pattern);
    }
    if (!query.exec()) {
        setError(query.lastError().text());
        return;
    }

    beginResetModel();
    m_tracks.clear();
    while (query.next()) {
        Track track;
        track.id = query.value(0).toInt();
        track.title = query.value(1).toString();
        track.artist = query.value(2).toString();
        track.album = query.value(3).toString();
        track.filePath = query.value(4).toString();
        track.sourceUrl = query.value(5).toString();
        track.source = query.value(6).toString();
        track.format = query.value(7).toString();
        track.coverPath = query.value(8).toString();
        track.duration = query.value(9).toLongLong();
        track.bitrate = query.value(10).toInt();
        track.rating = query.value(11).toInt();
        track.playCount = query.value(12).toInt();
        track.lastPlayed = query.value(13).toLongLong();
        track.skipCount = query.value(14).toInt();
        track.listenMs = query.value(15).toLongLong();
        m_tracks.append(track);
    }
    endResetModel();
    emit countChanged();
    emit libraryChanged();
}

void LibraryModel::setError(const QString &error) {
    if (error == m_lastError)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}
