#include "importmanager.h"

#include "librarymodel.h"
#include "settingsstore.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QTimer>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>

// Реализация импорта: очередь файлов, проверка расширений, копирование и добавление.

ImportManager::ImportManager(LibraryModel* library, SettingsStore* settings, QObject* parent)
    : QObject(parent), m_library(library), m_settings(settings)
{}

void ImportManager::importFiles(const QList<QUrl>& fileUrls)
{
    QStringList paths;
    // Берём только локальные файлы, отбрасывая остальные URL.
    for (const QUrl& url : fileUrls) {
        if (url.isLocalFile())
            paths.append(url.toLocalFile());
    }
    importPaths(paths);
}

void ImportManager::importPaths(const QStringList& paths)
{
    if (m_busy)
        return;
    if (paths.isEmpty()) {
        setError(QStringLiteral("Выберите хотя бы один аудиофайл."));
        return;
    }

    setBusy(true);
    setError({});
    m_pending = paths;
    m_added = 0;
    m_skipped = 0;
    // Обрабатываем через очередь событий, чтобы UI успевал перерисовываться.
    QTimer::singleShot(0, this, &ImportManager::importNext);
}

void ImportManager::importNext()
{
    if (m_pending.isEmpty()) {
        setStatus(QStringLiteral("Добавлено: %1 · пропущено: %2").arg(m_added).arg(m_skipped));
        setBusy(false);
        emit importCompleted(m_added, m_skipped);
        return;
    }
    const QString path = m_pending.takeFirst();
    setStatus(QStringLiteral("Добавляем %1 · осталось %2").arg(QFileInfo(path).fileName()).arg(m_pending.size()));
    // Список поддерживаемых аудиоформатов.
    const QSet<QString> supported{"mp3", "flac", "wav", "ogg", "opus", "m4a", "aac", "wma", "aiff", "aif"};
    if (!supported.contains(QFileInfo(path).suffix().toLower())) {
        ++m_skipped;
    } else {
        const QString stored = copyToLibrary(path);
        if (stored.isEmpty() || !m_library->addTrack(stored))
            ++m_skipped;
        else
            ++m_added;
    }
    QTimer::singleShot(0, this, &ImportManager::importNext);
}

void ImportManager::attachFile(int trackId, const QUrl& url)
{
    if (m_busy || !url.isLocalFile())
        return;
    const QString path = copyToLibrary(url.toLocalFile());
    if (path.isEmpty() || !m_library->attachFile(trackId, path))
        setError("Не удалось привязать файл. Возможно, он уже есть в медиатеке.");
    else {
        setError({});
        setStatus("Аудиофайл привязан к треку");
    }
}

void ImportManager::importFolder(const QUrl& folderUrl)
{
    const QString folder = folderUrl.isLocalFile() ? folderUrl.toLocalFile() : folderUrl.toString();
    if (!QDir(folder).exists()) {
        setError(QStringLiteral("Папка не найдена: %1").arg(folder));
        return;
    }
    const QSet<QString> extensions = {QStringLiteral("mp3"), QStringLiteral("flac"), QStringLiteral("wav"),
                                      QStringLiteral("ogg"), QStringLiteral("opus"), QStringLiteral("m4a"),
                                      QStringLiteral("aac"), QStringLiteral("wma")};
    QStringList paths;
    // Рекурсивно обходим папку и собираем файлы поддерживаемых форматов.
    QDirIterator it(folder, QDir::Files | QDir::Readable, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = it.next();
        if (extensions.contains(QFileInfo(path).suffix().toLower()))
            paths.append(path);
    }
    paths.sort();
    importPaths(paths);
}

QString ImportManager::copyToLibrary(const QString& source)
{
    const QFileInfo info(source);
    if (!info.exists() || !info.isFile())
        return {};
    const QString root = QDir::cleanPath(m_settings->libraryFolder());
    const QString absolute = QDir::cleanPath(info.absoluteFilePath());
    // Файл уже лежит в библиотечной папке — копировать не нужно.
    if (absolute == root || absolute.startsWith(root + QDir::separator()))
        return absolute;

    QDir().mkpath(root);
    QString target = QDir(root).filePath(info.fileName());
    int suffix = 1;
    // При совпадении имени подбираем уникальное (… (2), … (3)), а одинаковые
    // по содержимому файлы не дублируем — возвращаем существующий путь.
    while (QFileInfo::exists(target)) {
        if (QFileInfo(target).size() == info.size()) {
            QFile original(absolute), candidate(target);
            if (original.open(QIODevice::ReadOnly) && candidate.open(QIODevice::ReadOnly)) {
                QCryptographicHash a(QCryptographicHash::Sha256), b(QCryptographicHash::Sha256);
                if (a.addData(&original) && b.addData(&candidate) && a.result() == b.result())
                    return target;
            }
        }
        target = QDir(root).filePath(
            QStringLiteral("%1 (%2).%3").arg(info.completeBaseName()).arg(suffix++).arg(info.suffix()));
    }
    if (!QFile::copy(absolute, target)) {
        setError(QStringLiteral("Не удалось скопировать %1").arg(info.fileName()));
        return {};
    }
    return target;
}

void ImportManager::setBusy(bool busy)
{
    if (m_busy == busy)
        return;
    m_busy = busy;
    emit busyChanged();
}

void ImportManager::setStatus(const QString& status)
{
    if (m_statusText == status)
        return;
    m_statusText = status;
    emit statusTextChanged();
}

void ImportManager::setError(const QString& error)
{
    if (m_lastError == error)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}
