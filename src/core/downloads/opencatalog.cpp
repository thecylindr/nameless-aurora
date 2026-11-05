#include "opencatalog.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QRegularExpression>
#include <QUrlQuery>
#include <QFileInfo>
#include <cmath>

// Реализация каталога Internet Archive: двухэтапный запрос (поиск → метаданные) и разбор файлов.
OpenCatalog::OpenCatalog(QObject* parent, QNetworkAccessManager* transport)
    : QObject(parent), m_network(transport ? transport : new QNetworkAccessManager(this))
{}
void OpenCatalog::cancel()
{
    ++m_generation;
    const auto replies = m_replies;
    m_replies.clear();
    for (auto* reply : replies) {
        disconnect(reply, nullptr, this, nullptr);
        reply->abort();
        reply->deleteLater();
    }
}
void OpenCatalog::search(const QString& title, const QString& artist)
{
    cancel();
    m_tracks.clear();
    // Экранируем текст для запроса Lucene, чтобы спецсимволы не сломали поисковое выражение.
    auto quote = [](QString text) {
        text = text.left(180);
        text.replace('\\', "\\\\");
        text.replace('"', "\\\"");
        return '"' + text + '"';
    };
    QUrl url("https://archive.org/advancedsearch.php");
    QUrlQuery q;
    q.addQueryItem("q", "mediatype:audio AND (title:" + quote(title) +
                            (artist.isEmpty() ? QString() : " OR creator:" + quote(artist)) + ")");
    q.addQueryItem("fl[]", "identifier");
    q.addQueryItem("rows", "6");
    q.addQueryItem("output", "json");
    url.setQuery(q);
    request(url, false);
}
void OpenCatalog::request(const QUrl& url, bool metadata)
{
    QNetworkRequest httpRequest(url);
    httpRequest.setTransferTimeout(12000);
    httpRequest.setRawHeader("User-Agent", "AuroraPlayer/4.0 (music library)");
    auto* reply = m_network->get(httpRequest);
    m_replies.insert(reply);
    const int generation = m_generation;
    connect(reply, &QNetworkReply::readyRead, this, [reply] {
        if (reply->bytesAvailable() > 4 * 1024 * 1024)
            reply->abort();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, metadata, generation] {
        m_replies.remove(reply);
        reply->deleteLater();
        if (generation != m_generation)
            return;
        const bool ok = reply->error() == QNetworkReply::NoError;
        const auto bytes = reply->readAll();
        const auto root = bytes.size() <= 4 * 1024 * 1024 ? QJsonDocument::fromJson(bytes).object() : QJsonObject{};
        if (!metadata) {
            // Первый этап — поиск идентификаторов; затем для каждого (не больше 6) запрашиваем метаданные.
            if (!ok || root.isEmpty()) {
                emit failed("Internet Archive не ответил. Повтори поиск позже.");
                return;
            }
            for (const auto& entry : root.value("response").toObject().value("docs").toArray()) {
                const auto id = entry.toObject().value("identifier").toString();
                if (QRegularExpression("^[A-Za-z0-9_.-]{1,150}$").match(id).hasMatch() && m_replies.size() < 6)
                    request(QUrl("https://archive.org/metadata/" + id), true);
            }
        } else if (ok)
            m_tracks.append(parseFiles(root));
        if (m_replies.isEmpty())
            emit ready(m_tracks);
    });
}
QVariantList OpenCatalog::parseFiles(const QJsonObject& root)
{
    QVariantList result;
    const auto metadata = root.value("metadata").toObject();
    const auto flag = [](const QJsonValue& v) {
        return v.toBool() || v.toInt() == 1 || v.toString() == "true" || v.toString() == "1";
    };
    if (flag(root.value("is_dark")) || flag(metadata.value("nodownload")) || flag(root.value("is_access_restricted")))
        return result;
    const auto id = metadata.value("identifier").toString();
    if (!QRegularExpression("^[A-Za-z0-9_.-]{1,150}$").match(id).hasMatch())
        return result;
    const auto creator = metadata.value("creator");
    const auto creators = creator.toArray();
    QString artist =
        creator.isArray() ? (creators.isEmpty() ? QString() : creators.first().toString()) : creator.toString();
    QSet<QString> seen;
    for (const auto& entry : root.value("files").toArray()) {
        const auto file = entry.toObject();
        const auto name = file.value("name").toString();
        if (flag(file.value("private")) || name.split('/').contains("..") || name.startsWith('/') ||
            !name.endsWith(".mp3", Qt::CaseInsensitive))
            continue;
        const auto title = file.value("title").toString(QFileInfo(name).completeBaseName());
        if (seen.contains(title))
            continue;
        seen.insert(title);
        // Длительность может быть «секунды» или «ММ:СС» — поддерживаем оба формата.
        const auto length = file.value("length").toVariant().toString();
        double seconds = length.toDouble();
        if (length.contains(':')) {
            seconds = 0;
            for (const auto& part : length.split(':'))
                seconds = seconds * 60 + part.toDouble();
        }
        if (!std::isfinite(seconds) || seconds < 0 || seconds > 7 * 24 * 3600)
            seconds = 0;
        // Кодируем имя файла в URL, оставляя слэши (вложенные папки) как есть.
        const auto url =
            "https://archive.org/download/" + id + "/" + QString::fromUtf8(QUrl::toPercentEncoding(name, "/"));
        result.append(QVariantMap{{"title", title},
                                  {"artist", file.value("creator").toString(artist)},
                                  {"durationMs", qRound64(qMax(0., seconds) * 1000)},
                                  {"url", url},
                                  {"mediaUrl", url},
                                  {"mediaVerified", true},
                                  {"coverUrl", "https://archive.org/services/img/" + id}});
        if (result.size() == 12)
            break;
    }
    return result;
}
