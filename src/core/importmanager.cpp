#include "importmanager.h"

#include "librarymodel.h"
#include "settingsstore.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>

ImportManager::ImportManager(LibraryModel *library, SettingsStore *settings, QObject *parent)
    : QObject(parent), m_library(library), m_settings(settings) {
}

void ImportManager::importFiles(const QList<QUrl> &fileUrls) {
    QStringList paths;
    for (const QUrl &url : fileUrls) {
        if (url.isLocalFile())
            paths.append(url.toLocalFile());
    }
    importPaths(paths);
}

void ImportManager::importPaths(const QStringList &paths) {
    if (m_busy)
        return;
    if (paths.isEmpty()) {
        setError(QStringLiteral("Выберите хотя бы один аудиофайл."));
        return;
    }

    setBusy(true);
    setError({});
    int added = 0;
    int skipped = 0;
    for (const QString &path : paths) {
        setStatus(QStringLiteral("Импорт: %1").arg(QFileInfo(path).fileName()));
        QCoreApplication::processEvents();
        const QString storedPath = copyToLibrary(path);
        if (storedPath.isEmpty() || !m_library->addTrack(storedPath))
            ++skipped;
        else
            ++added;
    }
    setStatus(added > 0
                  ? QStringLiteral("Добавлено треков: %1. Пропущено: %2.").arg(added).arg(skipped)
                  : QStringLiteral("Новых треков нет. Пропущено: %1.").arg(skipped));
    setBusy(false);
    emit importCompleted(added, skipped);
}

void ImportManager::importFolder(const QUrl &folderUrl) {
    const QString folder = folderUrl.isLocalFile() ? folderUrl.toLocalFile() : folderUrl.toString();
    if (!QDir(folder).exists()) {
        setError(QStringLiteral("Папка не найдена: %1").arg(folder));
        return;
    }
    const QSet<QString> extensions = {
        QStringLiteral("mp3"), QStringLiteral("flac"), QStringLiteral("wav"), QStringLiteral("ogg"),
        QStringLiteral("opus"), QStringLiteral("m4a"), QStringLiteral("aac"), QStringLiteral("wma")
    };
    QStringList paths;
    QDirIterator it(folder, QDir::Files | QDir::Readable, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = it.next();
        if (extensions.contains(QFileInfo(path).suffix().toLower()))
            paths.append(path);
    }
    paths.sort();
    importPaths(paths);
}

QString ImportManager::copyToLibrary(const QString &source) {
    const QFileInfo info(source);
    if (!info.exists() || !info.isFile())
        return {};
    const QString root = QDir::cleanPath(m_settings->libraryFolder());
    const QString absolute = QDir::cleanPath(info.absoluteFilePath());
    if (absolute == root || absolute.startsWith(root + QDir::separator()))
        return absolute;

    QDir().mkpath(root);
    QString target = QDir(root).filePath(info.fileName());
    int suffix = 1;
    while (QFileInfo::exists(target)) {
        if (QFileInfo(target).size() == info.size())
            return target;
        target = QDir(root).filePath(QStringLiteral("%1 (%2).%3")
                                         .arg(info.completeBaseName()).arg(suffix++).arg(info.suffix()));
    }
    if (!QFile::copy(absolute, target)) {
        setError(QStringLiteral("Не удалось скопировать %1").arg(info.fileName()));
        return {};
    }
    return target;
}

void ImportManager::setBusy(bool busy) {
    if (m_busy == busy)
        return;
    m_busy = busy;
    emit busyChanged();
}

void ImportManager::setStatus(const QString &status) {
    if (m_statusText == status)
        return;
    m_statusText = status;
    emit statusTextChanged();
}

void ImportManager::setError(const QString &error) {
    if (m_lastError == error)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}
