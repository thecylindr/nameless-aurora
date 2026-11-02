#pragma once

#include <QObject>
#include <QUrl>

class LibraryModel;
class SettingsStore;

class ImportManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    ImportManager(LibraryModel *library, SettingsStore *settings, QObject *parent = nullptr);

    bool busy() const { return m_busy; }
    QString statusText() const { return m_statusText; }
    QString lastError() const { return m_lastError; }
    LibraryModel *library() const { return m_library; }

    Q_INVOKABLE void importFiles(const QList<QUrl> &fileUrls);
    Q_INVOKABLE void importPaths(const QStringList &paths);
    Q_INVOKABLE void importFolder(const QUrl &folderUrl);

signals:
    void busyChanged();
    void statusTextChanged();
    void lastErrorChanged();
    void importCompleted(int added, int skipped);

private:
    void setBusy(bool busy);
    void setStatus(const QString &status);
    void setError(const QString &error);
    QString copyToLibrary(const QString &source);

    LibraryModel *m_library;
    SettingsStore *m_settings;
    bool m_busy = false;
    QString m_statusText;
    QString m_lastError;
};
