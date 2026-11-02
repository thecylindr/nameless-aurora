#pragma once

#include <QObject>
#include <QString>

class SettingsStore;

class StorageManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qint64 freeBytes READ freeBytes NOTIFY freeBytesChanged)
    Q_PROPERTY(qint64 totalBytes READ totalBytes NOTIFY totalBytesChanged)
    Q_PROPERTY(qint64 libraryBytes READ libraryBytes NOTIFY libraryBytesChanged)
    Q_PROPERTY(QString libraryFolder READ libraryFolder NOTIFY libraryFolderChanged)

public:
    explicit StorageManager(SettingsStore *settings, QObject *parent = nullptr);

    qint64 freeBytes() const { return m_freeBytes; }
    qint64 totalBytes() const { return m_totalBytes; }
    qint64 libraryBytes() const { return m_libraryBytes; }
    QString libraryFolder() const { return m_libraryFolder; }

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QString formatBytes(qint64 bytes) const;
    Q_INVOKABLE qint64 estimateForDuration(qint64 durationMs, const QString &quality) const;
    Q_INVOKABLE bool canFit(qint64 estimatedBytes) const;

signals:
    void freeBytesChanged();
    void totalBytesChanged();
    void libraryBytesChanged();
    void libraryFolderChanged();

private:
    SettingsStore *m_settings;
    qint64 m_freeBytes = 0;
    qint64 m_totalBytes = 0;
    qint64 m_libraryBytes = 0;
    QString m_libraryFolder;
};
