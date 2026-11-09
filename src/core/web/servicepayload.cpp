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
    auto identity = value;
    if (value.isObject()) {
        const auto object = value.toObject();
        identity = object.value("id").isUndefined() ? object.value("trackId") : object.value("id");
    }
    const QString id = identity.toVariant().toString();
    const auto match = QRegularExpression("^([0-9]{1,20})(?::[0-9]{1,20})?$").match(id);
    return match.hasMatch() ? match.captured(1) : QString();
}
} // namespace
QString ServicePayload::yandexTrackUrl(const QString& id)
{
    return "https://music.yandex.ru/track/" + id;
}
QString ServicePayload::canonicalYandexTrackUrl(const QUrl& url)
{
    if ((url.scheme() != "https" && url.scheme() != "http") || !url.userInfo().isEmpty() ||
        !QRegularExpression("^music\\.yandex\\.(ru|com|by|kz|uz)$").match(url.host()).hasMatch())
        return {};
    const auto match = QRegularExpression("(?:^|/)track/([0-9]{1,20})/?$").match(url.path());
    return match.hasMatch() ? yandexTrackUrl(match.captured(1)) : QString();
}
QVector<Track> ServicePayload::yandexWave(const QJsonValue& payload)
{
    const auto object = payload.toObject();
    const auto result = object.contains("result") ? object.value("result").toObject() : object;
    QJsonArray tracks;
    for (const auto& entry : result.value("sequence").toArray()) {
        const auto track = entry.toObject().value("track").toObject();
        if (track.value("available").toBool(true) && !track.value("title").toString().trimmed().isEmpty())
            tracks.append(track);
        if (tracks.size() >= 50)
            break;
    }
    return yandex(QJsonObject{{"tracks", tracks}}).tracks;
}
ServicePayload::Collection ServicePayload::yandex(const QJsonValue& payload)
{
    Collection out;
    QJsonValue root = payload;
    // Некоторые ответы оборачивают данные в {"result": ...} — снимаем обёртку.
    if (root.isObject() && root.toObject().contains("result"))
        root = root.toObject().value("result");
    const auto envelope = root.toObject();
    const auto nested = envelope.value("playlist").toObject();
    const auto object = nested.contains("tracks") || nested.contains("trackIds") ? nested : envelope;
    out.playlistUuid = object.value("playlistUuid").toString(nested.value("playlistUuid").toString());
    if (!QRegularExpression("^[A-Za-z0-9_.-]{1,100}$").match(out.playlistUuid).hasMatch())
        out.playlistUuid.clear();
    out.title = object.value("title").toString(nested.value("title").toString());
    out.ownerId = identifier(object.value("owner").toObject().value("uid"));
    if (out.ownerId.isEmpty())
        out.ownerId = identifier(object.value("ownerId"));
    out.kind = identifier(object.value("kind"));
    // A per-page count must never hide a larger total from the enclosing pager.
    for (const auto& holder : {envelope, nested}) {
        for (const auto& count : {holder.value("trackCount"), holder.value("total"), holder.value("totalCount"),
                                  holder.value("summary").toObject().value("count"),
                                  holder.value("pager").toObject().value("total"),
                                  holder.value("paging").toObject().value("total")}) {
            bool valid = false;
            const auto number = count.toVariant().toString().toInt(&valid);
            if (valid && number >= 0) {
                out.totalKnown = true;
                out.total = qMax(out.total, qMin(number, 100001));
            }
        }
    }
    const auto ids = object.value("trackIds").toArray();
    out.manifest = object.value("trackIds").isArray();
    if (out.manifest) {
        out.total = qMax(out.total, int(ids.size()));
        out.totalKnown = true;
        QSet<QString> ordered;
        for (const auto& value : ids) {
            const auto id = identifier(value);
            if (!id.isEmpty() && !ordered.contains(id)) {
                ordered.insert(id);
                out.order.append(yandexTrackUrl(id));
            }
        }
    }
    out.fullPlaylist = out.totalKnown && (object.value("tracks").isArray() || out.manifest);
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
        if (!out.manifest)
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
