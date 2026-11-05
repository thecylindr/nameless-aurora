#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include "data/libraryrepository.h"
class OAuthManager;
class WebLoginManager;

// Загрузка плейлистов и треков из VK: через официальный audio.get API (токен) или через парсинг веб-страниц (вход в браузере).
class VkProvider : public QObject
{
    Q_OBJECT
  public:
    explicit VkProvider(OAuthManager* auth, WebLoginManager* webLogin, QObject* parent = nullptr);
    // Разбирает ссылку vk.com и запускает импорт треков подходящим способом (API или веб-сессия).
    void fetch(const QString& urlText);
    // Разбирает JSON-ответ API audio.get; в error — текст ошибки, в title — имя плейлиста.
    static QVector<Track> parseAudioGet(const QByteArray& json, QString* error, QString* title = nullptr);
    // Достаёт треки из атрибутов data-audio HTML-страницы VK.
    static QVector<Track> parseDataAudio(const QString& html);
    // Разбирает ответ AJAX load_section (al_audio.php) в список треков, имя и общее число.
    static QVector<Track> parseLoadSection(const QByteArray& body, QString* name = nullptr, QString* error = nullptr,
                                           qint64* total = nullptr);
  signals:
    void result(const QVector<Track>& tracks, const QString& name, const QString& error);

  private:
    // Запрашивает страницу плейлиста через API audio.get.
    void request(const QString& ownerId, const QString& albumId, const QString& accessKey, int page);
    // Грузит страницу VK через веб-сессию и парсит встроенные аудио-данные.
    void fetchWeb(const QUrl& url);
    // Подгружает порцию треков через AJAX load_section.
    void postLoadSection(const QString& ownerId, const QString& type, const QString& accessHash, int offset, int page);
    OAuthManager* m_auth;
    WebLoginManager* m_webLogin;
    QNetworkAccessManager m_network;
    QVector<Track> m_tracks;
    QString m_name;
    QString m_ownerId, m_albumId, m_accessKey;
    QString m_sectionOwner, m_sectionType, m_sectionHash;
    qint64 m_total = 0;
};
