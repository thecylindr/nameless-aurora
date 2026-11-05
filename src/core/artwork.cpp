#include "librarymodel.h"
#include <QBuffer>
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImageReader>
#include <QNetworkReply>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTimer>
#include <utility>

// Работа с обложками треков: импорт, кэширование удалённых картинок и их уведомления.
namespace
{
// Каталог, в котором хранятся извлечённые и скачанные обложки.
QString artworkFolder()
{
    const auto folder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/covers";
    QDir().mkpath(folder);
    return folder;
}
// Проверяет и сжимает картинку до 640 px, сохраняя в JPG. Отклоняет слишком большие файлы.
bool saveImage(const QByteArray& bytes, const QString& path)
{
    if (bytes.isEmpty() || bytes.size() > 5 * 1024 * 1024)
        return false;
    QBuffer buffer;
    buffer.setData(bytes);
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer);
    const auto size = reader.size();
    if (!size.isValid() || size.width() > 8192 || size.height() > 8192)
        return false;
    reader.setAutoTransform(true);
    reader.setScaledSize(size.scaled(640, 640, Qt::KeepAspectRatio));
    const auto image = reader.read();
    if (image.isNull())
        return false;
    QSaveFile file(path);
    return file.open(QIODevice::WriteOnly) && image.save(&file, "JPG", 90) && file.commit();
}
// Упаковывает данные трека в словарь для передачи в QML.
QVariantMap details(const Track& t)
{
    return {{"trackId", t.id},          {"title", t.title},         {"artist", t.artist},
            {"album", t.album},         {"url", t.sourceUrl},       {"source", t.source},
            {"durationMs", t.duration}, {"coverPath", t.coverPath}, {"downloaded", QFileInfo::exists(t.filePath)}};
}
} // namespace
// Возвращает детали одного трека по его ID (или пустую карту, если трека нет).
QVariantMap LibraryModel::trackDetails(int id) const
{
    const auto t = m_repository.track(id);
    return t.id ? details(t) : QVariantMap{};
}
// Возвращает детали всех треков медиатеки.
QVariantList LibraryModel::allTrackDetails() const
{
    QVariantList result;
    for (const auto& t : m_repository.tracks())
        result.append(details(t));
    return result;
}
// Обновляет метаданные трека и перечитывает модель.
bool LibraryModel::updateMetadata(int id, const QString& title, const QString& artist, const QString& album)
{
    if (!m_repository.updateMetadata(id, title, artist, album)) {
        setError(m_repository.error());
        return false;
    }
    setError({});
    refresh();
    return true;
}
// Импортирует локальную обложку, давая файлу имя на основе SHA-256 содержимого.
QString LibraryModel::importCover(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 5 * 1024 * 1024)
        return {};
    const auto bytes = file.readAll();
    const auto target =
        artworkFolder() + "/custom-" + QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex() + ".jpg";
    return saveImage(bytes, target) ? target : QString();
}
// Назначает обложку треку из локального файла или URL.
bool LibraryModel::setTrackCover(int id, const QUrl& url)
{
    const auto path = url.isLocalFile() ? importCover(url.toLocalFile()) : QString();
    if (path.isEmpty() || !m_repository.track(id).id) {
        setError("Выбери изображение JPG, PNG или WebP размером до 5 МБ.");
        return false;
    }
    const bool ok = m_repository.setCover(id, path);
    if (!ok)
        setError(m_repository.error());
    else {
        setError({});
        refresh();
    }
    return ok;
}
// Переносит список треков в альбом и при необходимости ставит общую обложку.
bool LibraryModel::setAlbumTracks(const QString& previous, const QString& title, const QVariantList& ids,
                                  const QUrl& cover)
{
    QString path;
    if (!cover.isEmpty()) {
        path = cover.isLocalFile() ? importCover(cover.toLocalFile()) : QString();
        if (path.isEmpty()) {
            setError("Не удалось прочитать обложку альбома.");
            return false;
        }
    }
    if (!m_repository.setAlbumTracks(previous, title, ids)) {
        setError(m_repository.error());
        return false;
    }
    if (!path.isEmpty())
        for (const auto& id : ids)
            m_repository.setCover(id.toInt(), path);
    setError({});
    refresh();
    return true;
}
// Ставит в очередь скачивание удалённой обложки (не более трёх параллельно).
void LibraryModel::cacheCover(int id, const QUrl& url)
{
    const auto track = m_repository.track(id);
    if (!track.id || QFileInfo::exists(track.coverPath) || url.scheme() != "https" || url.host().isEmpty() ||
        !url.userInfo().isEmpty())
        return;
    const auto key = url.toString();
    const auto path = artworkFolder() + "/remote-" +
                      QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha256).toHex() + ".jpg";
    if (QFileInfo::exists(path)) {
        m_repository.setCover(id, path);
        notifyCovers({id});
        return;
    }
    if (!m_coverTargets.contains(key))
        m_coverQueue.enqueue(key);
    if (!m_coverTargets[key].contains(id))
        m_coverTargets[key].append(id);
    fetchNextCover();
}
// Запускает скачивание очередной обложки, пока активных запросов меньше трёх.
void LibraryModel::fetchNextCover()
{
    while (m_coverActive < 3 && !m_coverQueue.isEmpty()) {
        const auto key = m_coverQueue.dequeue();
        ++m_coverActive;
        QNetworkRequest request{QUrl(key)};
        request.setTransferTimeout(12000);
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        auto* reply = m_coverNetwork.get(request);
        connect(reply, &QNetworkReply::readyRead, reply, [reply] {
            if (reply->bytesAvailable() > 5 * 1024 * 1024)
                reply->abort();
        });
        connect(reply, &QNetworkReply::finished, this, [this, reply, key] {
            --m_coverActive;
            const auto ids = m_coverTargets.take(key);
            const auto path = artworkFolder() + "/remote-" +
                              QCryptographicHash::hash(key.toUtf8(), QCryptographicHash::Sha256).toHex() + ".jpg";
            const bool ok = reply->error() == QNetworkReply::NoError && saveImage(reply->readAll(), path);
            reply->deleteLater();
            if (ok) {
                for (int id : ids) {
                    const auto t = m_repository.track(id);
                    if (t.id && !QFileInfo::exists(t.coverPath))
                        m_repository.setCover(id, path);
                }
                notifyCovers(ids);
            }
            fetchNextCover();
        });
    }
}
// Накопительно уведомляет модель об изменившихся обложках (пачка за 50 мс).
void LibraryModel::notifyCovers(const QList<int>& ids)
{
    for (int id : ids)
        m_changedCovers.insert(id);
    if (m_coverNotificationPending)
        return;
    m_coverNotificationPending = true;
    QTimer::singleShot(50, this, [this] {
        m_coverNotificationPending = false;
        const auto changed = std::exchange(m_changedCovers, QSet<int>{});
        for (int id : changed) {
            const int row = rowForTrackId(id);
            if (row < 0)
                continue;
            m_tracks[row].coverPath = m_repository.track(id).coverPath;
            emit dataChanged(index(row), index(row), {CoverPathRole});
        }
        emit libraryChanged();
    });
}
