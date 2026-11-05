#pragma once
// Поэтапный импорт аудиофайлов в медиатеку с копированием в библиотечную папку.

#include <QObject>
#include <QUrl>

class LibraryModel;
class SettingsStore;

class ImportManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

  public:
    ImportManager(LibraryModel* library, SettingsStore* settings, QObject* parent = nullptr);

    bool busy() const
    {
        return m_busy;
    }
    QString statusText() const
    {
        return m_statusText;
    }
    QString lastError() const
    {
        return m_lastError;
    }
    LibraryModel* library() const
    {
        return m_library;
    }

    // Импортирует файлы из списка URL (локальные).
    Q_INVOKABLE void importFiles(const QList<QUrl>& fileUrls);
    // Импортирует файлы из списка локальных путей.
    Q_INVOKABLE void importPaths(const QStringList& paths);
    // Привязывает аудиофайл к существующему треку.
    Q_INVOKABLE void attachFile(int trackId, const QUrl& fileUrl);
    // Рекурсивно импортирует все поддерживаемые файлы из папки.
    Q_INVOKABLE void importFolder(const QUrl& folderUrl);

  signals:
    void busyChanged();
    void statusTextChanged();
    void lastErrorChanged();
    void importCompleted(int added, int skipped);

  private:
    // Обрабатывает по одному файлу за такт, чтобы не блокировать UI.
    void importNext();
    QStringList m_pending;
    int m_added = 0, m_skipped = 0;
    void setBusy(bool busy);
    void setStatus(const QString& status);
    void setError(const QString& error);
    // Копирует файл в библиотечную папку (или возвращает уже существующий путь).
    QString copyToLibrary(const QString& source);

    LibraryModel* m_library;
    SettingsStore* m_settings;
    bool m_busy = false;
    QString m_statusText;
    QString m_lastError;
};
