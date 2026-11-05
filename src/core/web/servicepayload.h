#pragma once
#include "../data/libraryrepository.h"
#include <QJsonValue>
#include <QStringList>

// Разбор ответов веб-API сервисов (Яндекс Музыка и др.) в коллекции треков и ссылки на воспроизведение.
namespace ServicePayload
{
struct Collection
{
    QVector<Track> tracks;
    QStringList order;
    QString playlistUuid, title;
    int total = 0;
    bool fullPlaylist = false;
};
// Предпросмотр веб-коллекции, плейлист (полный или только идентификаторы) и ответы POST /tracks.
Collection yandex(const QJsonValue& payload);
// Формирует каноническую ссылку на трек Яндекс Музыки по его идентификатору.
QString yandexTrackUrl(const QString& id);
} // namespace ServicePayload
