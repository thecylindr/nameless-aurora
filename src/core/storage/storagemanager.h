#pragma once

#include <QObject>
#include <QString>

class SettingsStore;

// Управление хранилищем: считает свободное/занятое место на диске и размер библиотеки, оценивает вместимость загрузок.
class StorageManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(qint64 freeBytes READ freeBytes NOTIFY freeBytesChanged)
    Q_PROPERTY(qint64 totalBytes READ totalBytes NOTIFY totalBytesChanged)
    Q_PROPERTY(qint64 libraryBytes READ libraryBytes NOTIFY libraryBytesChanged)
    Q_PROPERTY(QString libraryFolder READ libraryFolder NOTIFY libraryFolderChanged)

  public:
    explicit StorageManager(SettingsStore* settings, QObject* parent = nullptr);

    qint64 freeBytes() const
    {
        return m_freeBytes;
    }
    bool busy() const
    {
        return m_scanning;
    }
    qint64 totalBytes() const
    {
        return m_totalBytes;
    }
    qint64 libraryBytes() const
    {
        return m_libraryBytes;
    }
    QString libraryFolder() const
    {
        return m_libraryFolder;
    }

    // Пересчитывает показатели диска и размер библиотеки (асинхронно, в фоновом потоке).
    Q_INVOKABLE void refresh();
    // Форматирует число байт в человекочитаемую строку (B, KiB, MiB, …).
    Q_INVOKABLE QString formatBytes(qint64 bytes) const;
    // Оценивает размер файла в байтах по длительности и выбранному качеству.
    Q_INVOKABLE qint64 estimateForDuration(qint64 durationMs, const QString& quality) const;
    // Проверяет, поместится ли загрузка с учётом резерва свободного места.
    Q_INVOKABLE bool canFit(qint64 estimatedBytes) const;

  signals:
    void busyChanged();
    void freeBytesChanged();
    void totalBytesChanged();
    void libraryBytesChanged();
    void libraryFolderChanged();

  private:
    SettingsStore* m_settings;
    qint64 m_freeBytes = 0;
    qint64 m_totalBytes = 0;
    qint64 m_libraryBytes = 0;
    QString m_libraryFolder;
    bool m_scanning = false, m_rescan = false;
};
