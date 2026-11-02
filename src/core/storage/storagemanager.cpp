#include "storagemanager.h"

#include "settingsstore.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QLocale>
#include <QStorageInfo>

#include <limits>

StorageManager::StorageManager(SettingsStore *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
{
    if (m_settings) {
        connect(m_settings, &SettingsStore::libraryFolderChanged, this, [this] {
            const QString folder = QDir::cleanPath(m_settings->libraryFolder());
            if (m_libraryFolder != folder) {
                m_libraryFolder = folder;
                emit libraryFolderChanged();
            }
            refresh();
        });
    }
    refresh();
}

void StorageManager::refresh()
{
    const QString folder = QDir::cleanPath(m_settings ? m_settings->libraryFolder() : QString());
    if (m_libraryFolder != folder) {
        m_libraryFolder = folder;
        emit libraryFolderChanged();
    }

    QStorageInfo storage(folder.isEmpty() ? QDir::rootPath() : folder);
    storage.refresh();
    const qint64 freeBytes = storage.isValid() && storage.isReady() ? qMax<qint64>(0, storage.bytesAvailable()) : 0;
    const qint64 totalBytes = storage.isValid() && storage.isReady() ? qMax<qint64>(0, storage.bytesTotal()) : 0;

    qint64 libraryBytes = 0;
    if (!folder.isEmpty() && QDir(folder).exists()) {
        QDirIterator iterator(folder, QDir::Files | QDir::Readable | QDir::NoDotAndDotDot,
                              QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            iterator.next();
            const QFileInfo info = iterator.fileInfo();
            if (info.isSymLink() || !info.isFile()) {
                continue;
            }
            const qint64 size = qMax<qint64>(0, info.size());
            if (libraryBytes > std::numeric_limits<qint64>::max() - size) {
                libraryBytes = std::numeric_limits<qint64>::max();
                break;
            }
            libraryBytes += size;
        }
    }

    if (m_freeBytes != freeBytes) {
        m_freeBytes = freeBytes;
        emit freeBytesChanged();
    }
    if (m_totalBytes != totalBytes) {
        m_totalBytes = totalBytes;
        emit totalBytesChanged();
    }
    if (m_libraryBytes != libraryBytes) {
        m_libraryBytes = libraryBytes;
        emit libraryBytesChanged();
    }
}

QString StorageManager::formatBytes(qint64 bytes) const
{
    static const QStringList units = {
        QStringLiteral("B"), QStringLiteral("KiB"), QStringLiteral("MiB"),
        QStringLiteral("GiB"), QStringLiteral("TiB"), QStringLiteral("PiB")
    };
    const bool negative = bytes < 0;
    double value = negative ? -static_cast<double>(bytes) : static_cast<double>(bytes);
    int unit = 0;
    while (value >= 1024.0 && unit < units.size() - 1) {
        value /= 1024.0;
        ++unit;
    }
    const int precision = unit == 0 || value >= 100.0 ? 0 : value >= 10.0 ? 1 : 2;
    QString result = QLocale().toString(value, 'f', precision);
    if (negative) {
        result.prepend(QLatin1Char('-'));
    }
    return result + QLatin1Char(' ') + units.at(unit);
}

qint64 StorageManager::estimateForDuration(qint64 durationMs, const QString &quality) const
{
    if (durationMs <= 0) {
        return 0;
    }
    const QString normalized = quality.trimmed().toLower();
    const qint64 bitrate = normalized == QStringLiteral("compact") ? 128
        : normalized == QStringLiteral("balanced") ? 192 : 320;
    const long double estimate = static_cast<long double>(durationMs) * bitrate / 8.0L;
    if (estimate >= std::numeric_limits<qint64>::max()) {
        return std::numeric_limits<qint64>::max();
    }
    return static_cast<qint64>(estimate);
}

bool StorageManager::canFit(qint64 estimatedBytes) const
{
    if (estimatedBytes < 0) {
        return false;
    }
    const qint64 reserve = qMax<qint64>(100LL * 1024LL * 1024LL, estimatedBytes / 20);
    return estimatedBytes <= m_freeBytes && reserve <= m_freeBytes - estimatedBytes;
}
