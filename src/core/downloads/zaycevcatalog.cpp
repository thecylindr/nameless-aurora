#include "zaycevcatalog.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QRegularExpression>
#include <cmath>

// Реализация каталога Zaycev.net: запросы к API поиска/разрешения и разбор ответов.
namespace
{
// Приводит значение к безопасному HTTPS-URL и отбрасывает ссылки с встроенными учётными данными.
QString https(const QString& value)
{
    if (value.isEmpty())
        return {};
    const QUrl u = QUrl("https://zaycev.net/").resolved(QUrl(value));
    return u.scheme() == "https" && !u.host().isEmpty() && u.userInfo().isEmpty() ? u.toString() : QString();
}
// Длительность может приходить как «секунды» или «ММ:СС».
qint64 duration(const QJsonValue& value)
{
    const auto text = value.toVariant().toString();
    double seconds = 0;
    if (text.contains(':')) {
        for (const auto& part : text.split(':'))
            seconds = seconds * 60 + part.toDouble();
    } else
        seconds = text.toDouble();
    return std::isfinite(seconds) && seconds > 0 && seconds < 604800 ? qRound64(seconds * 1000) : 0;
}
} // namespace
ZaycevCatalog::ZaycevCatalog(QObject* parent) : QObject(parent), m_network(this)
{}
void ZaycevCatalog::cancel()
{
    ++m_generation;
    const auto replies = m_replies;
    m_replies.clear();
    for (auto* r : replies) {
        disconnect(r, nullptr, this, nullptr);
        r->abort();
        r->deleteLater();
    }
}
void ZaycevCatalog::search(const QString& query)
{
    cancel();
    QUrl url("https://zaycev.net/api/external/pages/search/tracks");
    QUrlQuery q;
    q.addQueryItem("q", query.left(250));
    q.addQueryItem("page", "1");
    q.addQueryItem("limit", "12");
    url.setQuery(q);
    request(url);
}
void ZaycevCatalog::resolve(const QVariantMap& candidate)
{
    cancel();
    const auto hash = candidate.value("downloadHash", candidate.value("streamingHash")).toString();
    if (hash.isEmpty() || hash.size() > 2048) {
        emit failed("Zaycev не предоставил ссылку на аудио.");
        return;
    }
    // Если доступно скачивание — используем download, иначе — поток play.
    const auto method = candidate.value("downloadHash").toString().isEmpty() ? "play/" : "download/";
    request(QUrl("https://zaycev.net/api/external/track/" + QString(method) +
                 QString::fromUtf8(QUrl::toPercentEncoding(hash))),
            candidate);
}
void ZaycevCatalog::request(const QUrl& url, const QVariantMap& candidate)
{
    QNetworkRequest request(url);
    request.setTransferTimeout(6500);
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("User-Agent", "Mozilla/5.0 AuroraPlayer/4.0");
    request.setRawHeader("Referer", "https://zaycev.net/");
    auto* reply = m_network.get(request);
    m_replies.insert(reply);
    const int generation = m_generation;
    connect(reply, &QNetworkReply::readyRead, this, [reply] {
        if (reply->bytesAvailable() > 4 * 1024 * 1024)
            reply->abort();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, generation, candidate] {
        m_replies.remove(reply);
        reply->deleteLater();
        if (generation != m_generation)
            return;
        const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const auto bytes = reply->readAll();
        // Ошибка 423 означает, что сервис ограничил скачивание этой записи.
        if (reply->error() != QNetworkReply::NoError || bytes.size() > 4 * 1024 * 1024) {
            emit failed(code == 423 ? "Zaycev ограничил загрузку этой записи."
                                    : "Прямой запрос Zaycev не завершился; используем страницу сайта.");
            return;
        }
        const auto json = QJsonDocument::fromJson(bytes);
        const auto object = json.object();
        if (candidate.isEmpty()) {
            const auto tracks = parse(object);
            if (tracks.isEmpty()) {
                emit failed("В ответе Zaycev нет доступных вариантов; проверяем страницу.");
                return;
            }
            emit ready(tracks);
            return;
        }
        QString media = object.value("url").toString();
        const auto data = object.value("data");
        if (media.isEmpty())
            media = data.isString() ? data.toString() : data.toObject().value("url").toString();
        // Некоторые ответы отдают ссылку как «голую» JSON-строку — оборачиваем её в массив.
        if (media.isEmpty() && bytes.trimmed().startsWith('"')) {
            const auto array = QJsonDocument::fromJson('[' + bytes + ']').array();
            if (!array.isEmpty())
                media = array.first().toString();
        }
        media = https(media);
        if (media.isEmpty()) {
            emit failed("Zaycev не отдал аудиофайл этой записи.");
            return;
        }
        auto result = candidate;
        result["mediaUrl"] = media;
        result["mediaVerified"] = true;
        result["userAgent"] = "Mozilla/5.0 AuroraPlayer/4.0";
        emit resolved(result);
    });
}
QVariantList ZaycevCatalog::parse(const QJsonObject& payload)
{
    auto root = payload;
    if (!root.contains("tracksInfo") && root.value("data").isObject())
        root = root.value("data").toObject();
    if (root.value("tracks").isObject())
        root = root.value("tracks").toObject();
    const auto info = root.value("tracksInfo");
    const auto objects = info.toObject();
    auto ids = root.value("trackIds").toArray();
    if (ids.isEmpty())
        for (auto i = objects.begin(); i != objects.end(); ++i)
            ids.append(i.key());
    QVariantList results;
    for (int position = 0; position < ids.size(); ++position) {
        const auto value = ids.at(position);
        const auto id = value.toVariant().toString();
        if (!QRegularExpression("^[0-9]{1,18}$").match(id).hasMatch())
            continue;
        auto track = objects.value(id).toObject();
        if (track.isEmpty())
            for (const auto& v : info.toArray())
                if (v.toObject().value("id").toVariant().toString() == id) {
                    track = v.toObject();
                    break;
                }
        if (track.isEmpty() && info.isArray() && info.toArray().size() == ids.size() &&
            !info.toArray().at(position).toObject().contains("id"))
            track = info.toArray().at(position).toObject();
        const auto title = track.value("track").toString();
        if (title.isEmpty())
            continue;
        QVariantMap t{
            {"title", title},
            {"artist", track.value("artistName").toString()},
            {"durationMs", duration(track.value("duration"))},
            // Формируем URL страницы трека: id разбивается на подкаталог и файл (.shtml).
            {"url", "https://zaycev.net/pages/" + (id.size() > 2 ? id.left(id.size() - 2) : "0") + "/" + id + ".shtml"},
            {"coverUrl", https(track.value("imageJpg").toString(track.value("imageWebp").toString()))}};
        if (track.value("download").isString() && !track.value("download").toString().isEmpty())
            t["downloadHash"] = track.value("download").toString();
        if (track.value("streaming").isString() && !track.value("streaming").toString().isEmpty())
            t["streamingHash"] = track.value("streaming").toString();
        results.append(t);
        if (results.size() == 12)
            break;
    }
    return results;
}
