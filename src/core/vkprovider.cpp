#include "vkprovider.h"
#include "web/vktext.h"
#include <QSet>
#include "auth/oauthmanager.h"
#include "auth/webloginmanager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrlQuery>

// Реализация VkProvider: два пути получения треков — официальный audio.get API
// (при наличии токена) и веб-парсинг страниц VK с AJAX-подгрузкой секций (при входе через браузер).

namespace
{
// Максимальный размер страницы audio.get — VK не отдаёт больше 6000 треков за один запрос.
const int pageSize = 6000;
// Подмена User-Agent под Chrome, чтобы VK отдавал полную разметку, пригодную для парсинга.
const QByteArray chromeUa =
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/130.0.0.0 Safari/537.36";
QString audioAccessError()
{
    return QStringLiteral(
        "У приложения VK нет доступа к аудио (VK открыл audio API только для верифицированных приложений). "
        "Импортируйте плейлист файлом TXT/CSV/JSON или используйте токен приложения с доступом к аудио.");
}
QString unsupportedError()
{
    return QStringLiteral("Поддерживаются ссылки вида vk.com/audios…, /music/playlist/…, /music/album/…");
}
QString sessionError()
{
    return QStringLiteral("Сессия VK устарела или страница недоступна. Обновите вход: «Войти в VK».");
}
QString loginPromptError()
{
    return QStringLiteral(
        "Для VK нужен вход: нажмите «Войти в VK» на странице импорта (вход через браузер) или подключите токен.");
}
// Разворачивает HTML-сущности, встречающиеся внутри атрибутов data-audio.
QString htmlUnescape(QString text)
{
    text.replace(QStringLiteral("&quot;"), QStringLiteral("\""));
    text.replace(QStringLiteral("&amp;"), QStringLiteral("&"));
    text.replace(QStringLiteral("&lt;"), QStringLiteral("<"));
    text.replace(QStringLiteral("&gt;"), QStringLiteral(">"));
    text.replace(QStringLiteral("&#39;"), QStringLiteral("'"));
    text.replace(QStringLiteral("&apos;"), QStringLiteral("'"));
    return text;
}
// Собирает Track из «плоского» массива VK: [0]=id, [1]=owner_id, [3]=название, [4]=исполнитель,
// [5]=длительность в секундах (переводится в миллисекунды).
Track trackFromArray(const QJsonArray& item)
{
    Track track;
    track.artist = item.at(4).toString();
    track.title = item.at(3).toString();
    track.duration = qRound64(item.at(5).toDouble() * 1000);
    track.source = QStringLiteral("VK");
    if (!track.title.isEmpty())
        track.sourceUrl = QStringLiteral("https://vk.com/audio%1_%2")
                              .arg(item.at(1).toVariant().toLongLong())
                              .arg(item.at(0).toVariant().toLongLong());
    return track;
}
} // namespace

VkProvider::VkProvider(OAuthManager* auth, WebLoginManager* webLogin, QObject* parent)
    : QObject(parent), m_auth(auth), m_webLogin(webLogin)
{}

void VkProvider::fetch(const QString& urlText)
{
    m_tracks.clear();
    m_name.clear();
    m_ownerId.clear();
    m_albumId.clear();
    m_accessKey.clear();
    m_sectionOwner.clear();
    m_sectionType.clear();
    m_sectionHash.clear();
    m_total = 0;
    const QUrl url(urlText.trimmed());
    QString host = url.host().toLower();
    if (host.startsWith(QStringLiteral("www.")))
        host = host.mid(4);
    if (host != QStringLiteral("vk.com") && host != QStringLiteral("vk.ru") &&
        !host.endsWith(QStringLiteral(".vk.com")) && !host.endsWith(QStringLiteral(".vk.ru"))) {
        emit result({}, {}, unsupportedError());
        return;
    }
    const QString path = url.path();
    // Три поддерживаемых вида ссылок: /audios<id>, /music/playlist/... и /music/album/...
    static const QRegularExpression audios(QStringLiteral("^/audios(-?\\d+)$"));
    static const QRegularExpression playlistRe(QStringLiteral("^/music/playlist/(-?\\d+)_(\\d+)$"));
    static const QRegularExpression albumRe(QStringLiteral("^/music/album/(\\d+)_(-?\\d+)$"));
    QRegularExpressionMatch match = audios.match(path);
    if (match.hasMatch())
        m_ownerId = match.captured(1);
    else if ((match = playlistRe.match(path)).hasMatch()) {
        m_ownerId = match.captured(1);
        m_albumId = match.captured(2);
    } else if ((match = albumRe.match(path)).hasMatch()) {
        m_albumId = match.captured(1);
        m_ownerId = match.captured(2);
    } else {
        emit result({}, {}, unsupportedError());
        return;
    }
    m_accessKey = QUrlQuery(url).queryItemValue(QStringLiteral("access_key"));
    const QString token = m_auth ? m_auth->vkToken() : QString();
    // Приоритет — официальный API; иначе пытаемся использовать веб-сессию, иначе просим войти.
    if (!token.isEmpty()) {
        request(m_ownerId, m_albumId, m_accessKey, 0);
        return;
    }
    if (m_webLogin && m_webLogin->hasSession(QStringLiteral("vk"))) {
        fetchWeb(url);
        return;
    }
    emit result({}, {}, loginPromptError());
}

void VkProvider::request(const QString& ownerId, const QString& albumId, const QString& accessKey, int page)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("owner_id"), ownerId);
    if (!albumId.isEmpty())
        query.addQueryItem(QStringLiteral("album_id"), albumId);
    if (!accessKey.isEmpty())
        query.addQueryItem(QStringLiteral("access_key"), accessKey);
    query.addQueryItem(QStringLiteral("count"), QString::number(pageSize));
    query.addQueryItem(QStringLiteral("offset"), QString::number(page * pageSize));
    query.addQueryItem(QStringLiteral("access_token"), m_auth->vkToken());
    query.addQueryItem(QStringLiteral("v"), QStringLiteral("5.199"));
    QNetworkRequest httpRequest(
        QUrl(QStringLiteral("https://api.vk.com/method/audio.get?") + query.query(QUrl::FullyEncoded)));
    httpRequest.setTransferTimeout(30000);
    auto* reply = m_network.get(httpRequest);
    connect(reply, &QNetworkReply::finished, this, [this, reply, page] {
        const auto bytes = reply->readAll();
        reply->deleteLater();
        QString error, title;
        const auto tracks = parseAudioGet(bytes, &error, &title);
        if (!error.isEmpty()) {
            emit result({}, {}, error);
            return;
        }
        if (page == 0 && !title.isEmpty())
            m_name = title;
        m_tracks += tracks;
        // Подгружаем не более трёх страниц (до ~18k треков), чтобы не зависать на гигантских плейлистах.
        if (tracks.size() == pageSize && page < 2)
            request(m_ownerId, m_albumId, m_accessKey, page + 1);
        else
            emit result(m_tracks, m_name.isEmpty() ? QStringLiteral("VK Музыка") : m_name, {});
    });
}

void VkProvider::fetchWeb(const QUrl& url)
{
    QNetworkRequest httpRequest(QUrl(QStringLiteral("https://vk.com") + url.path()));
    httpRequest.setRawHeader("User-Agent", chromeUa);
    const QString cookies =
        m_webLogin ? m_webLogin->cookieHeader(QStringLiteral("vk"), QUrl(QStringLiteral("https://vk.com"))) : QString();
    if (!cookies.isEmpty())
        httpRequest.setRawHeader("Cookie", cookies.toUtf8());
    httpRequest.setTransferTimeout(30000);
    auto* reply = m_network.get(httpRequest);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const auto bytes = reply->readAll();
        const QString host = reply->url().host().toLower();
        reply->deleteLater();
        if (host.startsWith(QStringLiteral("login.")) || host.contains(QStringLiteral("login.vk"))) {
            emit result({}, {}, sessionError());
            return;
        }
        const QString html = QString::fromUtf8(bytes);
        const auto tracks = parseDataAudio(html);
        if (!tracks.isEmpty()) {
            emit result(tracks, m_name.isEmpty() ? QStringLiteral("VK Музыка") : m_name, {});
            return;
        }
        // Если в разметке есть access_hash, подгружаем секцию через AJAX load_section.
        if (html.contains(QStringLiteral("access_hash"))) {
            const int anchor = html.indexOf(QStringLiteral("initial_playlist"));
            // Ищем атрибуты владельца/типа/хэша только рядом с началом плейлиста, чтобы не захватить чужие.
            const QString scope = anchor >= 0 ? html.mid(anchor, 6000) : html;
            static const QRegularExpression ownerRe(QStringLiteral("data-owner=\"(-?\\d+)\""));
            static const QRegularExpression typeRe(QStringLiteral("data-type=\"([a-zA-Z_]+)\""));
            static const QRegularExpression hashRe(QStringLiteral("data-access-hash=\"([^\"]+)\""));
            const auto owner = ownerRe.match(scope);
            const auto type = typeRe.match(scope);
            const auto hash = hashRe.match(scope);
            if (owner.hasMatch() && type.hasMatch() && hash.hasMatch()) {
                m_sectionOwner = owner.captured(1);
                m_sectionType = type.captured(1) == QStringLiteral("audios") ? QStringLiteral("audios")
                                                                             : QStringLiteral("playlist");
                m_sectionHash = hash.captured(1);
                postLoadSection(m_sectionOwner, m_sectionType, m_sectionHash, 0, 0);
                return;
            }
        }
        emit result({}, {}, sessionError());
    });
}

void VkProvider::postLoadSection(const QString& ownerId, const QString& type, const QString& accessHash, int offset,
                                 int page)
{
    QUrlQuery form;
    form.addQueryItem(QStringLiteral("act"), QStringLiteral("load_section"));
    form.addQueryItem(QStringLiteral("al"), QStringLiteral("1"));
    form.addQueryItem(QStringLiteral("owner_id"), ownerId);
    form.addQueryItem(QStringLiteral("type"), type);
    form.addQueryItem(QStringLiteral("access_hash"), accessHash);
    form.addQueryItem(QStringLiteral("from_id"), ownerId);
    form.addQueryItem(QStringLiteral("offset"), QString::number(offset));
    QNetworkRequest httpRequest(QUrl(QStringLiteral("https://vk.com/al_audio.php")));
    httpRequest.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    httpRequest.setRawHeader("X-Requested-With", "XMLHttpRequest");
    httpRequest.setRawHeader("User-Agent", chromeUa);
    const QString cookies =
        m_webLogin ? m_webLogin->cookieHeader(QStringLiteral("vk"), QUrl(QStringLiteral("https://vk.com"))) : QString();
    if (!cookies.isEmpty())
        httpRequest.setRawHeader("Cookie", cookies.toUtf8());
    httpRequest.setTransferTimeout(30000);
    auto* reply = m_network.post(httpRequest, form.query(QUrl::FullyEncoded).toUtf8());
    connect(reply, &QNetworkReply::finished, this, [this, reply, offset, page] {
        const auto bytes = reply->readAll();
        const bool ok = reply->error() == QNetworkReply::NoError;
        reply->deleteLater();
        if (!ok) {
            emit result({}, {}, sessionError());
            return;
        }
        QString error, name;
        qint64 total = 0;
        const auto tracks = parseLoadSection(bytes, &name, &error, &total);
        if (!error.isEmpty()) {
            emit result({}, {}, error);
            return;
        }
        if (page == 0 && !name.isEmpty())
            m_name = name;
        if (page == 0)
            m_total = total;
        m_tracks += tracks;
        // Продолжаем подгрузку, пока не вытянули все треки (не более трёх страниц).
        if (!tracks.isEmpty() && m_tracks.size() < m_total && page < 2)
            postLoadSection(m_sectionOwner, m_sectionType, m_sectionHash, offset + tracks.size(), page + 1);
        else
            emit result(m_tracks, m_name.isEmpty() ? QStringLiteral("VK Музыка") : m_name, {});
    });
}

QVector<Track> VkProvider::parseAudioGet(const QByteArray& json, QString* error, QString* title)
{
    error->clear();
    if (title)
        title->clear();
    QJsonParseError parse;
    const auto document = QJsonDocument::fromJson(json, &parse);
    if (parse.error != QJsonParseError::NoError || !document.isObject()) {
        *error = QStringLiteral("VK вернул некорректный ответ.");
        return {};
    }
    const auto root = document.object();
    if (root.contains(QStringLiteral("error"))) {
        int code = 0;
        QString message;
        if (root.value(QStringLiteral("error")).isObject()) {
            const auto object = root.value(QStringLiteral("error")).toObject();
            code = object.value(QStringLiteral("error_code")).toInt();
            message = object.value(QStringLiteral("error_msg")).toString();
        } else {
            code = root.value(QStringLiteral("error_code")).toInt();
            message = root.value(QStringLiteral("error_msg")).toString();
        }
        // Код 5 — недействительный токен; 201/203/204 — нет доступа к аудио (API открыт только верифицированным приложениям).
        if (code == 5)
            *error = QStringLiteral("Токен VK недействителен — войдите заново.");
        else if (code == 201 || code == 203 || code == 204 ||
                 message.contains(QStringLiteral("доступ к аудио"), Qt::CaseInsensitive))
            *error = audioAccessError();
        else if (!message.isEmpty())
            *error = QStringLiteral("VK: %1").arg(message);
        else
            *error = QStringLiteral("VK вернул ошибку %1.").arg(code);
        return {};
    }
    const auto response = root.value(QStringLiteral("response")).toObject();
    if (title)
        *title = response.value(QStringLiteral("title")).toString();
    QVector<Track> tracks;
    for (const auto& value : response.value(QStringLiteral("items")).toArray()) {
        const auto object = value.toObject();
        Track track;
        track.artist = object.value(QStringLiteral("artist")).toString();
        track.title = object.value(QStringLiteral("title")).toString();
        track.duration = qRound64(object.value(QStringLiteral("duration")).toDouble() * 1000);
        track.source = QStringLiteral("VK");
        track.sourceUrl = QStringLiteral("https://vk.com/audio%1_%2")
                              .arg(object.value(QStringLiteral("owner_id")).toVariant().toLongLong())
                              .arg(object.value(QStringLiteral("id")).toVariant().toLongLong());
        if (!track.title.isEmpty())
            tracks.append(track);
    }
    return tracks;
}

QVector<Track> VkProvider::parseDataAudio(const QString& html)
{
    QVector<Track> tracks;
    QSet<QString> seen;
    // data-audio хранит JSON-массив в одинарных или двойных кавычках (обратная ссылка \1).
    const QRegularExpression attribute(R"(data-audio\s*=\s*(["'])(.*?)\1)",
                                       QRegularExpression::DotMatchesEverythingOption);
    auto matches = attribute.globalMatch(html);
    while (matches.hasNext()) {
        const auto match = matches.next();
        QJsonParseError parse;
        const auto document = QJsonDocument::fromJson(htmlUnescape(match.captured(2)).toUtf8(), &parse);
        if (parse.error != QJsonParseError::NoError || !document.isArray())
            continue;
        const auto array = document.array();
        // Один трек приходит как [a,b,...], а несколько — как [[...],[...]]; нормализуем к списку.
        const QJsonArray entries = !array.isEmpty() && !array.first().isArray() ? QJsonArray{array} : array;
        for (const auto& value : entries) {
            const auto item = value.toArray();
            if (item.size() < 6)
                continue;
            auto track = trackFromArray(item);
            if (track.title.isEmpty() || seen.contains(track.sourceUrl))
                continue;
            seen.insert(track.sourceUrl);
            tracks.append(track);
        }
    }
    return tracks;
}

QVector<Track> VkProvider::parseLoadSection(const QByteArray& body, QString* name, QString* error, qint64* total)
{
    if (name)
        name->clear();
    if (error)
        error->clear();
    if (total)
        *total = 0;
    QJsonParseError parse;
    const auto document = QJsonDocument::fromJson(VkText::decode(body).toUtf8(), &parse);
    if (parse.error != QJsonParseError::NoError || !document.isObject()) {
        if (error)
            *error = sessionError();
        return {};
    }
    const auto payload = document.object().value(QStringLiteral("payload")).toArray();
    // Ответ load_section: payload[1] — массив секции, где [0] — список, [1] — имя плейлиста.
    if (payload.size() < 2) {
        if (error)
            *error = sessionError();
        return {};
    }
    const auto section = payload.at(1).toArray();
    if (section.isEmpty()) {
        if (error)
            *error = sessionError();
        return {};
    }
    const auto list = section.at(0).toArray();
    if (list.size() < 2) {
        if (error)
            *error = sessionError();
        return {};
    }
    // Первый элемент списка — общее число треков; далее идут сами треки.
    if (total)
        *total = list.at(0).toVariant().toLongLong();
    if (name && section.size() > 1)
        *name = section.at(1).toString();
    QVector<Track> tracks;
    for (int i = 1; i < list.size(); ++i) {
        const auto item = list.at(i).toArray();
        if (item.size() < 6)
            continue;
        const auto track = trackFromArray(item);
        if (!track.title.isEmpty())
            tracks.append(track);
    }
    if (tracks.isEmpty() && error)
        *error = sessionError();
    return tracks;
}
