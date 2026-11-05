#include "servicepayload.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>

namespace
{
// Возвращает идентификатор трека, только если это строка из 1–20 цифр (защита от произвольных полей API).
QString identifier(const QJsonValue& value)
{
    const QString id = value.toVariant().toString();
    return QRegularExpression("^[0-9]{1,20}$").match(id).hasMatch() ? id : QString();
}
} // namespace
QString ServicePayload::yandexTrackUrl(const QString& id)
{
    return "https://music.yandex.ru/track/" + id;
}
ServicePayload::Collection ServicePayload::yandex(const QJsonValue& payload)
{
    Collection out;
    QJsonValue root = payload;
    // Некоторые ответы оборачивают данные в {"result": ...} — снимаем обёртку.
    if (root.isObject() && root.toObject().contains("result"))
        root = root.toObject().value("result");
    const auto object = root.toObject();
    const auto playlist = object.value("playlist").toObject();
    out.playlistUuid = object.value("playlistUuid").toString(playlist.value("playlistUuid").toString());
    // Допускаем только безопасный идентификатор плейлиста, иначе сбрасываем его.
    if (!QRegularExpression("^[A-Za-z0-9_.-]{1,100}$").match(out.playlistUuid).hasMatch())
        out.playlistUuid.clear();
    out.title = object.value("title").toString(playlist.value("title").toString());
    out.fullPlaylist = object.contains("trackCount") && object.value("tracks").isArray();
    // Общее число треков берём из trackCount (или summary.count) и ограничиваем сверху.
    out.total =
        qBound(0, object.value("trackCount").toInt(object.value("summary").toObject().value("count").toInt()), 100000);
    const auto entries = root.isArray() ? root.toArray() : object.value("tracks").toArray();
    QSet<QString> seen;
    for (const auto& entry : entries) {
        auto track = entry.toObject();
        // Запись может быть как самим треком, так и обёрткой {"track": {...}}.
        if (track.value("track").isObject())
            track = track.value("track").toObject();
        const auto id = identifier(track.value("id"));
        if (id.isEmpty() || seen.contains(id))
            continue;
        seen.insert(id);
        const auto url = yandexTrackUrl(id);
        out.order.append(url);
        if (track.value("title").toString().trimmed().isEmpty())
            continue;
        Track t;
        t.title = track.value("title").toString().trimmed();
        QStringList artists;
        for (const auto& artist : track.value("artists").toArray()) {
            const auto name = artist.toObject().value("name").toString().trimmed();
            if (!name.isEmpty())
                artists.append(name);
        }
        t.artist = artists.join(", ");
        const auto albums = track.value("albums").toArray();
        if (!albums.isEmpty())
            t.album = albums.first().toObject().value("title").toString();
        t.duration = qMax<qint64>(0, track.value("durationMs").toVariant().toLongLong());
        t.source = "Яндекс Музыка";
        t.sourceUrl = url;
        QString cover = track.value("coverUri").toString();
        if (!cover.isEmpty()) {
            if (!cover.startsWith("https://"))
                cover = "https://" + cover;
            // Шаблон %% в URI обложки заменяется на нужный размер.
            cover.replace("%%", "400x400");
            t.coverUrl = cover;
        }
        // Сохраняем только идентификатор воспроизведения, без произвольных полей API/аккаунта.
        t.webData = {{"url", url}, {"title", t.title}, {"artist", t.artist}, {"trackId", id}};
        out.tracks.append(t);
    }
    return out;
}
