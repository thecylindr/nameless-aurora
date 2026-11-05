#include "librarymodel.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QProcess>
#include <QNetworkReply>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QUuid>
#include <QtMath>
#include <algorithm>

// Реализация модели библиотеки: чтение метаданных, плейлисты, рекомендации,
// импорт обложек и синхронизация с репозиторием.

LibraryModel::LibraryModel(QObject* parent, const QString& databasePath)
    : QAbstractListModel(parent), m_repository(databasePath)
{
    if (!m_repository.ready())
        setError(m_repository.error());
    else
        refresh();
}
LibraryModel::~LibraryModel()
{
    // Прерываем незавершённые запросы скачивания обложек.
    for (auto* reply : m_coverNetwork.findChildren<QNetworkReply*>()) {
        disconnect(reply, nullptr, this, nullptr);
        reply->abort();
    }
}

int LibraryModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_tracks.size();
}

QVariant LibraryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tracks.size())
        return {};
    const Track& track = m_tracks.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return track.title;
    case IdRole:
        return track.id;
    case ArtistRole:
        return track.artist;
    case AlbumRole:
        return track.album;
    case FilePathRole:
        return track.filePath;
    case SourceUrlRole:
        return track.sourceUrl;
    case SourceRole:
        return track.source;
    case FormatRole:
        return track.format;
    case CoverPathRole:
        return track.coverPath;
    case DurationRole:
        return track.duration;
    case BitrateRole:
        return track.bitrate;
    case RatingRole:
        return track.rating;
    case PlayCountRole:
        return track.playCount;
    case LastPlayedRole:
        return track.lastPlayed;
    case SkipCountRole:
        return track.skipCount;
    case ListenMsRole:
        return track.listenMs;
    case AvailableRole:
        return !track.filePath.isEmpty() && QFileInfo::exists(track.filePath);
    default:
        return {};
    }
}

QHash<int, QByteArray> LibraryModel::roleNames() const
{
    return {{IdRole, "trackId"},          {TitleRole, "title"},           {ArtistRole, "artist"},
            {AlbumRole, "album"},         {FilePathRole, "filePath"},     {SourceUrlRole, "sourceUrl"},
            {SourceRole, "source"},       {FormatRole, "format"},         {CoverPathRole, "coverPath"},
            {DurationRole, "duration"},   {BitrateRole, "bitrate"},       {RatingRole, "rating"},
            {PlayCountRole, "playCount"}, {LastPlayedRole, "lastPlayed"}, {SkipCountRole, "skipCount"},
            {ListenMsRole, "listenMs"},   {AvailableRole, "available"}};
}

void LibraryModel::setSearchText(const QString& text)
{
    if (text == m_searchText)
        return;
    m_searchText = text;
    emit searchTextChanged();
    refresh();
}

// Читает метаданные аудиофайла через ffprobe и возвращает заполненный Track.
Track LibraryModel::probeFile(const QString& filePath, const QString& executable)
{
    Track track;
    const QFileInfo info(filePath);
    track.filePath = info.absoluteFilePath();
    track.title = info.completeBaseName();
    track.artist = QStringLiteral("Неизвестный исполнитель");
    track.format = info.suffix().toUpper();

    QProcess ffprobe;
    ffprobe.start(executable,
                  {QStringLiteral("-v"), QStringLiteral("quiet"), QStringLiteral("-print_format"),
                   QStringLiteral("json"), QStringLiteral("-show_format"), QStringLiteral("-show_streams"), filePath});
    if (!ffprobe.waitForStarted(1000))
        return track;
    if (!ffprobe.waitForFinished(8000)) {
        ffprobe.kill();
        ffprobe.waitForFinished(1000);
        return {};
    }
    if (ffprobe.exitStatus() != QProcess::NormalExit || ffprobe.exitCode() != 0)
        return {};

    const QJsonObject root = QJsonDocument::fromJson(ffprobe.readAllStandardOutput()).object();
    // Убеждаемся, что в файле есть хотя бы один аудиопоток.
    bool audio = false;
    for (const auto& stream : root.value("streams").toArray())
        if (stream.toObject().value("codec_type").toString() == "audio") {
            audio = true;
            break;
        }
    if (!audio)
        return {};
    const QJsonObject format = root.value(QStringLiteral("format")).toObject();
    const QJsonObject tags = format.value(QStringLiteral("tags")).toObject();
    // Теги встречаются и в нижнем, и в верхнем регистре — пробуем оба варианта.
    track.title =
        tags.value(QStringLiteral("title")).toString(tags.value(QStringLiteral("TITLE")).toString(track.title));
    track.artist =
        tags.value(QStringLiteral("artist")).toString(tags.value(QStringLiteral("ARTIST")).toString(track.artist));
    track.album = tags.value(QStringLiteral("album")).toString(tags.value(QStringLiteral("ALBUM")).toString());
    track.duration = qRound64(format.value(QStringLiteral("duration")).toString().toDouble() * 1000.0);
    track.bitrate = format.value(QStringLiteral("bit_rate")).toString().toInt() / 1000;
    // Если битрейт не указан — оцениваем его по размеру файла и длительности.
    if (track.bitrate == 0)
        track.bitrate = static_cast<int>(info.size() * 8 / qMax<qint64>(1, track.duration));
    return track;
}

bool LibraryModel::hasFile(const QString& path) const
{
    const QString canonical = QFileInfo(path).canonicalFilePath();
    return !canonical.isEmpty() && m_repository.hasFile(canonical);
}
// Добавляет трек из файла: считывает метаданные, сохраняет в БД и извлекает обложку.
bool LibraryModel::addTrack(const QString& filePath, const QString& sourceUrl, const QString& source)
{
    const QFileInfo info(filePath);
    if (!info.isFile() || hasFile(filePath))
        return false;
    Track track = probeFile(info.canonicalFilePath());
    if (track.title.isEmpty()) {
        setError("В файле не найден поддерживаемый аудиопоток: " + info.fileName());
        return false;
    }
    track.sourceUrl = sourceUrl;
    track.source = source.isEmpty() ? QStringLiteral("Local") : source;
    const int id = m_repository.saveTrack(track);
    if (!id) {
        setError(m_repository.error());
        return false;
    }
    if (track.title.isEmpty()) {
        setError("В файле не найден поддерживаемый аудиопоток.");
        return false;
    }
    track.coverPath = extractCover(track.filePath, id);
    if (!m_repository.attachFile(id, track)) {
        setError(m_repository.error());
        return false;
    }
    setError({});
    refresh();
    return true;
}
bool LibraryModel::attachFile(int id, const QString& path)
{
    if (!QFileInfo(path).isFile() || m_repository.track(id).id == 0)
        return false;
    Track track = probeFile(QFileInfo(path).canonicalFilePath());
    if (track.title.isEmpty()) {
        setError("В файле не найден поддерживаемый аудиопоток.");
        return false;
    }
    track.coverPath = extractCover(track.filePath, id);
    if (!m_repository.attachFile(id, track)) {
        setError(m_repository.error());
        return false;
    }
    setError({});
    refresh();
    return true;
}

bool LibraryModel::attachPreparedFile(int id, const QString& path, Track metadata)
{
    const auto previous = m_repository.track(id);
    if (!previous.id || !QFileInfo(path).isFile())
        return false;
    metadata.filePath = QFileInfo(path).canonicalFilePath();
    if (!previous.coverPath.isEmpty())
        metadata.coverPath = previous.coverPath;
    if (!m_repository.attachFile(id, metadata)) {
        setError(m_repository.error());
        return false;
    }
    setError({});
    refresh();
    return true;
}
// Извлекает первый видеокадр (обложку) из файла через ffmpeg, уменьшая до 640 px.
QString LibraryModel::extractCover(const QString& filePath, int trackId, const QString& executable)
{
    const QString coverDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/covers");
    QDir().mkpath(coverDir);
    const QString coverPath = QDir(coverDir).filePath(QStringLiteral("%1.jpg").arg(trackId));
    QProcess ffmpeg;
    ffmpeg.start(executable,
                 {QStringLiteral("-hide_banner"), QStringLiteral("-loglevel"), QStringLiteral("error"),
                  QStringLiteral("-y"), QStringLiteral("-i"), filePath, QStringLiteral("-map"), QStringLiteral("0:v:0"),
                  QStringLiteral("-frames:v"), QStringLiteral("1"), QStringLiteral("-vf"),
                  QStringLiteral("scale=640:640:force_original_aspect_ratio=decrease"), coverPath});
    if (!ffmpeg.waitForFinished(5000) || ffmpeg.exitCode() != 0 || !QFileInfo::exists(coverPath)) {
        QFile::remove(coverPath);
        return {};
    }
    return coverPath;
}

void LibraryModel::setRating(int id, int rating)
{
    if (!m_repository.setRating(id, rating))
        setError(m_repository.error());
    refresh();
}
void LibraryModel::removeTrack(int id)
{
    if (!m_repository.removeTrack(id))
        setError(m_repository.error());
    refresh();
}
void LibraryModel::incrementPlayCount(int id)
{
    if (!m_repository.recordPlay(id))
        setError(m_repository.error());
    emit libraryChanged();
}
void LibraryModel::recordListen(int id, qint64 milliseconds, bool skipped)
{
    if (!m_repository.recordListen(id, milliseconds, skipped))
        setError(m_repository.error());
    emit libraryChanged();
}
void LibraryModel::setPlaylistFilter(int id)
{
    if (m_playlistFilter == id)
        return;
    m_playlistFilter = id;
    emit filterChanged();
    refresh();
}
void LibraryModel::setSortOrder(const QString& order)
{
    if (m_sortOrder == order)
        return;
    m_sortOrder = order;
    emit filterChanged();
    refresh();
}
int LibraryModel::createPlaylist(const QString& name)
{
    const int id = m_repository.createPlaylist(name);
    if (!id)
        setError(m_repository.error());
    refresh();
    return id;
}
void LibraryModel::renamePlaylist(int id, const QString& name)
{
    if (!m_repository.renamePlaylist(id, name))
        setError(m_repository.error());
    refresh();
}
void LibraryModel::deletePlaylist(int id)
{
    if (!m_repository.deletePlaylist(id))
        setError(m_repository.error());
    if (m_playlistFilter == id) {
        m_playlistFilter = 0;
        emit filterChanged();
    }
    refresh();
}
void LibraryModel::addToPlaylist(int playlist, int track)
{
    if (!m_repository.addToPlaylist(playlist, track))
        setError(m_repository.error());
    refresh();
}
void LibraryModel::removeFromPlaylist(int playlist, int track)
{
    if (!m_repository.removeFromPlaylist(playlist, track))
        setError(m_repository.error());
    refresh();
}
QVector<int> LibraryModel::playableIds() const
{
    QVector<int> ids;
    for (const auto& t : m_tracks)
        if (!t.filePath.isEmpty() && QFileInfo::exists(t.filePath))
            ids.append(t.id);
    return ids;
}
QVariantList LibraryModel::queueDetails(const QVariantList& ids) const
{
    QVariantList result;
    for (const auto& id : ids) {
        const Track t = m_repository.track(id.toInt());
        if (t.id)
            result.append(QVariantMap{{"trackId", t.id},
                                      {"title", t.title},
                                      {"artist", t.artist},
                                      {"coverPath", t.coverPath},
                                      {"duration", t.duration}});
    }
    return result;
}

int LibraryModel::rowForTrackId(int id) const
{
    for (int row = 0; row < m_tracks.size(); ++row) {
        if (m_tracks.at(row).id == id)
            return row;
    }
    return -1;
}

QVariantList LibraryModel::recommendations(int limit) const
{
    QVariantList result;
    QVector<Track> tracks;
    QHash<QString, double> artistAffinity;
    // Считаем «аффинити» исполнителя по истории: рейтинг, прослушивания, пропуски.
    for (const Track& track : m_repository.tracks()) {
        if (track.filePath.isEmpty() || !QFileInfo::exists(track.filePath))
            continue;
        artistAffinity[track.artist] +=
            track.rating * 3.0 + track.playCount * 1.5 + track.listenMs / 3600000.0 - track.skipCount * 0.8;
        tracks.append(track);
    }

    struct ScoredTrack
    {
        Track track;
        double score;
    };
    QVector<ScoredTrack> scored;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (const Track& track : tracks) {
        const double hours = track.listenMs / 3600000.0;
        // Давность последнего прослушивания в днях (не слушали — считаем «давно»).
        const double daysAgo = track.lastPlayed > 0 ? (now - track.lastPlayed) / 86400000.0 : 3650.0;
        // Взвешенная оценка: рейтинг, число прослушиваний, время, пропуски и аффинити.
        double score = track.rating * 12.0 + qLn(1.0 + track.playCount) * 5.0 + hours * 2.0 - track.skipCount * 3.0 +
                       artistAffinity.value(track.artist) * 0.45;
        // Недавно прослушанные получают бонус, затухающий по экспоненте (период ~21 день).
        if (track.lastPlayed > 0)
            score += 8.0 * qExp(-daysAgo / 21.0);
        // Не прослушанные треки любимых исполнителей слегка «подталкиваем».
        if (track.playCount == 0 && artistAffinity.value(track.artist) > 0)
            score += 9.0;
        scored.append({track, score});
    }
    std::stable_sort(scored.begin(), scored.end(), [](const ScoredTrack& a, const ScoredTrack& b) {
        if (!qFuzzyCompare(a.score, b.score))
            return a.score > b.score;
        return a.track.title.compare(b.track.title, Qt::CaseInsensitive) < 0;
    });

    // Ограничиваем разнообразие: не более двух треков одного исполнителя.
    QHash<QString, int> artistCounts;
    const int maximum = qBound(1, limit, 50);
    for (const ScoredTrack& item : scored) {
        if (artistCounts.value(item.track.artist) >= 2)
            continue;
        const Track& track = item.track;
        result.append(
            QVariantMap{{QStringLiteral("trackId"), track.id},
                        {QStringLiteral("title"), track.title},
                        {QStringLiteral("artist"), track.artist},
                        {QStringLiteral("album"), track.album},
                        {QStringLiteral("filePath"), track.filePath},
                        {QStringLiteral("source"), track.source},
                        {QStringLiteral("format"), track.format},
                        {QStringLiteral("coverPath"), track.coverPath},
                        {QStringLiteral("duration"), track.duration},
                        {QStringLiteral("rating"), track.rating},
                        {QStringLiteral("playCount"), track.playCount},
                        {QStringLiteral("reason"), track.playCount == 0 ? QStringLiteral("Из вашей медиатеки")
                                                                        : QStringLiteral("По вашим прослушиваниям")}});
        ++artistCounts[track.artist];
        if (result.size() == maximum)
            break;
    }
    return result;
}

void LibraryModel::refresh()
{
    if (!m_repository.ready())
        return;
    beginResetModel();
    m_tracks = m_repository.tracks(m_searchText, m_playlistFilter, m_sortOrder);
    m_downloadedCount = 0;
    for (const auto& track : m_repository.tracks())
        if (!track.filePath.isEmpty() && QFileInfo(track.filePath).isFile())
            ++m_downloadedCount;
    endResetModel();
    emit countChanged();
    emit libraryChanged();
}

void LibraryModel::setError(const QString& error)
{
    if (error == m_lastError)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}
