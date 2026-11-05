#pragma once
// Импорт/экспорт медиатеки: плейлисты из Яндекс Музыки, VK и YouTube,
// а также переносимые архивы и текстовые списки.
#include <QObject>
#include <QUrl>
#include <QNetworkAccessManager>
#include "data/libraryrepository.h"
class LibraryModel;
class SettingsStore;
class DependencyManager;
class OAuthManager;
class WebLoginManager;
class VkProvider;
class QProcess;
class LibraryExchange : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(QString status READ status NOTIFY changed)
    Q_PROPERTY(QVariantList importedTracks READ importedTracks NOTIFY changed)
  public:
    LibraryExchange(LibraryModel*, SettingsStore*, DependencyManager*, OAuthManager*, WebLoginManager*,
                    QObject* parent = nullptr);
    ~LibraryExchange() override;
    bool busy() const
    {
        return m_busy;
    }
    QString status() const
    {
        return m_status;
    }
    QVariantList importedTracks() const
    {
        return m_importedTracks;
    }
    Q_INVOKABLE void cancel();
    // Импортирует список треков из локального файла (TXT/CSV/JSON или переносимый архив).
    Q_INVOKABLE void importList(const QUrl& file, const QString& source);
    // Импортирует плейлист по ссылке (Яндекс Музыка, YouTube, VK).
    Q_INVOKABLE void importUrl(const QString& url);
    // Импортирует «мою музыку» через существующий вход в браузере.
    Q_INVOKABLE void importMyMusic(const QString& service, const QString& profileId = {});
    // Обновляет «битые» названия VK-треков свежими из сервиса.
    Q_INVOKABLE void repairVkMetadata();
    // Импортирует из браузера (ссылка на плейлист сервиса).
    Q_INVOKABLE void importFromBrowser(const QString& url, const QString& profileId = {});
    // Сохраняет переносимую медиатеку (аудио + обложки + манифест) в папку.
    Q_INVOKABLE void exportPortable(const QUrl& folder, int playlist = 0);
    // Сохраняет простой текстовый список «исполнитель — название».
    Q_INVOKABLE void exportText(const QUrl& file, int playlist = 0);
    // Разбирает текст списка (JSON/CSV/построчно) в вектор треков.
    static QVector<Track> parseText(const QByteArray& data, const QString& extension, const QString& source,
                                    QString* error);
  signals:
    void changed();
    void completed(int playlistId);
    void audioImportRequested(const QVariantList& tracks);

  private:
    // Завершает операцию: сбрасывает флаги и рассылает статус/событие completed.
    void finish(const QString& status, int playlist = 0);
    // Сохраняет вектор треков в новый плейлист (в транзакции).
    void saveList(const QVector<Track>& tracks, const QString& name);
    void completeYandexPlaylist(class QJsonObject playlist);
    // Импортирует переносимый архив из манифеста.
    void importPortable(const QUrl& file, const class QJsonObject& manifest);
    // Получает плейлист Яндекс Музыки без токена (публичная ссылка).
    void importYandexLink(const QUrl& url);
    // Получает плейлист Яндекс Музыки с OAuth-токеном (с одним повтором при 401/403).
    void fetchYandexAuthed(const QUrl& url, const QString& token, bool retried);
    // Догружает недостающие треки плейлиста Яндекс Музыки отдельным запросом.
    void fetchYandexMissing(const QJsonObject& playlist, const QStringList& missing, const QString& token);
    // Возвращает заголовок Cookie для api Яндекс Музыки (если есть сессия браузера).
    QByteArray yandexCookieHeader() const;
    LibraryModel* m_library;
    SettingsStore* m_settings;
    DependencyManager* m_dependencies;
    OAuthManager* m_oauth;
    WebLoginManager* m_webLogin;
    VkProvider* m_vk;
    QNetworkAccessManager m_network;
    QProcess* m_process = nullptr;
    QVariantList m_importedTracks;
    bool m_nativeImport = false;
    bool m_downloadAfterImport = false;
    bool m_metadataOnly = false;
    bool m_busy = false;
    QString m_status;
};
