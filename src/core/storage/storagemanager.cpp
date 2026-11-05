#include "storagemanager.h"

#include "settingsstore.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QLocale>
#include <QStorageInfo>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>

#include <limits>

StorageManager::StorageManager(SettingsStore* settings, QObject* parent) : QObject(parent), m_settings(settings)
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

    // Если сканирование уже идёт, запоминаем, что нужен повторный проход по завершении.
    if (m_scanning) {
        m_rescan = true;
        return;
    }
    m_scanning = true;
    emit busyChanged();
    struct Snapshot
    {
        qint64 freeBytes, totalBytes, libraryBytes;
    };
    auto* watcher = new QFutureWatcher<Snapshot>(this);
    connect(watcher, &QFutureWatcher<Snapshot>::finished, this, [this, watcher, folder] {
        const auto result = watcher->result();
        watcher->deleteLater();
        // Применяем результат только если каталог за время сканирования не поменялся.
        if (folder == m_libraryFolder) {
            if (m_freeBytes != result.freeBytes) {
                m_freeBytes = result.freeBytes;
                emit freeBytesChanged();
            }
            if (m_totalBytes != result.totalBytes) {
                m_totalBytes = result.totalBytes;
                emit totalBytesChanged();
            }
            if (m_libraryBytes != result.libraryBytes) {
                m_libraryBytes = result.libraryBytes;
                emit libraryBytesChanged();
            }
        }
        m_scanning = false;
        emit busyChanged();
        if (m_rescan) {
            m_rescan = false;
            refresh();
        }
    });
    watcher->setFuture(QtConcurrent::run([folder] {
        QStorageInfo storage(folder.isEmpty() ? QDir::rootPath() : folder);
        storage.refresh();
        const qint64 freeBytes = storage.isValid() && storage.isReady() ? qMax<qint64>(0, storage.bytesAvailable()) : 0;
        const qint64 totalBytes = storage.isValid() && storage.isReady() ? qMax<qint64>(0, storage.bytesTotal()) : 0;

        // Суммируем размеры обычных файлов библиотеки, пропуская симлинки.
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
                // Защита от переполнения при суммировании.
                if (libraryBytes > std::numeric_limits<qint64>::max() - size) {
                    libraryBytes = std::numeric_limits<qint64>::max();
                    break;
                }
                libraryBytes += size;
            }
        }

        return Snapshot{freeBytes, totalBytes, libraryBytes};
    }));
}

QString StorageManager::formatBytes(qint64 bytes) const
{
    static const QStringList units = {QStringLiteral("B"),   QStringLiteral("KiB"), QStringLiteral("MiB"),
                                      QStringLiteral("GiB"), QStringLiteral("TiB"), QStringLiteral("PiB")};
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

qint64 StorageManager::estimateForDuration(qint64 durationMs, const QString& quality) const
{
    if (durationMs <= 0) {
        return 0;
    }
    const QString normalized = quality.trimmed().toLower();
    // Соответствие качества → битрейт (кбит/с): compact/balanced/прочее (по умолчанию 320).
    const qint64 bitrate = normalized == QStringLiteral("compact")    ? 128
                           : normalized == QStringLiteral("balanced") ? 192
                                                                      : 320;
    // Объём = длительность (мс) * битрейт (кбит/с) / 8 → байты.
    const long double estimate = static_cast<long double>(durationMs) * bitrate / 8.0L;
    if (estimate >= std::numeric_limits<qint64>::max()) {
        return std::numeric_limits<qint64>::max();
    }
    return static_cast<qint64>(estimate);
}

bool StorageManager::canFit(qint64 estimatedBytes) const
{
    // Пока идёт сканирование, берём данные напрямую из QStorageInfo, чтобы не ждать.
    if (m_scanning) {
        const QStorageInfo current(m_libraryFolder);
        const auto available = current.bytesAvailable();
        // Резерв — не менее 100 МБ или 5% от размера загрузки.
        const auto reserve = qMax<qint64>(100LL * 1024 * 1024, estimatedBytes / 20);
        return estimatedBytes >= 0 && current.isValid() && current.isReady() && estimatedBytes <= available &&
               reserve <= available - estimatedBytes;
    }
    if (estimatedBytes < 0) {
        return false;
    }
    const qint64 reserve = qMax<qint64>(100LL * 1024LL * 1024LL, estimatedBytes / 20);
    return estimatedBytes <= m_freeBytes && reserve <= m_freeBytes - estimatedBytes;
}
