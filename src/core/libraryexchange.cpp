#include "libraryexchange.h"
#include "web/vktext.h"
#include "web/servicepayload.h"
#include <QSettings>
#include "librarymodel.h"
#include "settingsstore.h"
#include "vkprovider.h"
#include "auth/oauthmanager.h"
#include "auth/webloginmanager.h"
#include "downloads/dependencymanager.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QProcess>
#include <QRegularExpression>
#include <QSaveFile>
#include <QTimer>
#include <QUrlQuery>
#include <QUuid>
#include <QtConcurrent>

// Реализация импорта/экспорта: разбор списков, запросы к сервисам,
// переносимые архивы и обновление метаданных VK.
namespace
{
// Максимальный размер ответа сервиса или файла списка (20 МБ).
constexpr qint64 maxBytes = 20 * 1024 * 1024;
// Преобразует JSON-объект сервиса в структуру Track, подставляя URL источника.
Track jsonTrack(QJsonObject o, const QString& source)
{
    if (o.value("track").isObject())
        o = o.value("track").toObject();
    Track t;
    t.title = o.value("title").toString(o.value("name").toString());
    t.artist = o.value("artist").toString(o.value("uploader").toString());
    if (o.value("artists").isArray()) {
        QStringList artists;
        for (const auto& v : o.value("artists").toArray())
            artists << v.toObject().value("name").toString();
        t.artist = artists.join(", ");
    }
    t.album = o.value("album").toString();
    if (o.value("albums").isArray() && !o.value("albums").toArray().isEmpty())
        t.album = o.value("albums").toArray().first().toObject().value("title").toString();
    t.source = source;
    t.sourceUrl = o.value("webpage_url").toString(o.value("url").toString());
    // Для Яндекса собираем каноническую ссылку на трек, чтобы потом найти его.
    if (source == "Яндекс Музыка" && !o.value("id").toVariant().toString().isEmpty()) {
        const auto albums = o.value("albums").toArray();
        QString albumId = albums.isEmpty() ? QString() : albums.first().toObject().value("id").toVariant().toString();
        t.sourceUrl = "https://music.yandex.ru/" + (albumId.isEmpty() ? QString() : "album/" + albumId + "/") +
                      "track/" + o.value("id").toVariant().toString();
    }
    t.duration = o.value("durationMs").toVariant().toLongLong();
    if (!t.duration)
        t.duration = qRound64(o.value("duration").toDouble() * 1000);
    return t;
}
// Возвращает абсолютный путь к файлу внутри каталога экспорта, отсекая
// выход за его пределы (защита от path traversal в импортируемом манифесте).
QString bundleFile(const QDir& root, const QString& relative)
{
    if (relative.isEmpty())
        return {};
    if (QDir::isAbsolutePath(relative))
        return {};
    QString path = QFileInfo(root.filePath(relative)).canonicalFilePath();
    if (path.isEmpty() || !path.startsWith(root.canonicalPath() + QDir::separator()) || !QFileInfo(path).isFile())
        return {};
    return path;
}
} // namespace
LibraryExchange::LibraryExchange(LibraryModel* l, SettingsStore* s, DependencyManager* d, OAuthManager* oauth,
                                 WebLoginManager* webLogin, QObject* p)
    : QObject(p), m_library(l), m_settings(s), m_dependencies(d), m_oauth(oauth), m_webLogin(webLogin)
{
    connect(m_library, &LibraryModel::libraryChanged, this, [this] {
        if (m_importedTracks.isEmpty())
            return;
        // Обновляем флаг «скачан» у импортированных треков по факту наличия файла.
        QSet<int> available;
        for (const auto& track : m_library->repository().tracks())
            if (QFileInfo::exists(track.filePath))
                available.insert(track.id);
        bool updated = false;
        for (auto& entry : m_importedTracks) {
            auto item = entry.toMap();
            const bool downloaded = available.contains(item.value("trackId").toInt());
            if (item.value("downloaded").toBool() != downloaded) {
                item["downloaded"] = downloaded;
                entry = item;
                updated = true;
            }
        }
        if (updated)
            emit changed();
    });
    if (m_webLogin) {
        connect(m_webLogin, &WebLoginManager::collectionReady, this,
                [this](const QVector<Track>& tracks, const QString& name, const QString& error) {
                    if (!m_nativeImport)
                        return;
                    m_nativeImport = false;
                    if (!error.isEmpty()) {
                        m_metadataOnly = false;
                        finish(error);
                        return;
                    }
                    if (m_metadataOnly) {
                        m_metadataOnly = false;
                        // Режим «починить названия VK»: обновляем только испорченные поля,
                        // не трогая аудиофайлы и плейлисты.
                        auto& repo = m_library->repository();
                        QHash<QString, Track> existing;
                        for (const auto& track : repo.tracks())
                            if (track.source == "VK")
                                existing.insert(track.sourceUrl, track);
                        if (!repo.transaction()) {
                            finish(repo.error());
                            return;
                        }
                        int changed = 0;
                        bool ok = true;
                        for (const auto& fresh : tracks) {
                            if (!existing.contains(fresh.sourceUrl))
                                continue;
                            const auto old = existing.value(fresh.sourceUrl);
                            // Берём свежее название, только если старое было «битым».
                            const auto fixed = [](const QString& before, const QString& after) {
                                return VkText::broken(before) && !after.isEmpty() && !VkText::broken(after) ? after
                                                                                                            : before;
                            };
                            const auto title = fixed(old.title, fresh.title), artist = fixed(old.artist, fresh.artist),
                                       album = fixed(old.album, fresh.album);
                            if (title == old.title && artist == old.artist && album == old.album)
                                continue;
                            if (!repo.updateMetadata(old.id, title, artist, album)) {
                                ok = false;
                                break;
                            }
                            ++changed;
                        }
                        if (!ok || !repo.commit()) {
                            repo.rollback();
                            finish("Названия не обновлены: " + repo.error());
                            return;
                        }
                        m_library->refresh();
                        finish(QString("Обновлено названий VK: %1. Аудиофайлы и плейлисты сохранены.").arg(changed));
                        return;
                    }
                    saveList(tracks, name);
                });
        connect(m_webLogin, &WebLoginManager::statusChanged, this, [this] {
            if (m_nativeImport) {
                m_status = m_webLogin->status();
                emit changed();
            }
        });
    }
    m_vk = new VkProvider(oauth, webLogin, this);
    connect(m_vk, &VkProvider::result, this,
            [this](const QVector<Track>& tracks, const QString& name, const QString& error) {
                if (!error.isEmpty())
                    finish(error);
                else
                    saveList(tracks, name);
            });
}
LibraryExchange::~LibraryExchange()
{
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}
// Завершает операцию: сбрасывает состояние и уведомляет UI о результате.
void LibraryExchange::finish(const QString& status, int playlist)
{
    m_metadataOnly = false;
    m_busy = false;
    m_status = status;
    if (!playlist)
        m_downloadAfterImport = false;
    emit changed();
    if (playlist)
        emit completed(playlist);
}

QVector<Track> LibraryExchange::parseText(const QByteArray& data, const QString& extension, const QString& source,
                                          QString* error)
{
    QVector<Track> tracks;
    QString text = QString::fromUtf8(data);
    // Убираем BOM, если он есть в начале файла.
    text.remove(QChar(0xfeff));
    if (extension == "json") {
        QJsonParseError parse;
        const auto document = QJsonDocument::fromJson(data, &parse);
        if (parse.error != QJsonParseError::NoError) {
            *error = "Некорректный JSON: " + parse.errorString();
            return {};
        }
        QJsonArray entries = document.array();
        if (document.isObject()) {
            auto root = document.object();
            if (root.value("result").isObject())
                root = root.value("result").toObject();
            if (root.value("playlist").isObject())
                root = root.value("playlist").toObject();
            entries = root.value("tracks").toArray();
            if (entries.isEmpty())
                entries = root.value("entries").toArray();
            if (entries.isEmpty() && root.contains("title"))
                entries.append(root);
        }
        for (const auto& v : entries) {
            auto t = jsonTrack(v.toObject(), source);
            if (!t.title.isEmpty())
                tracks.append(t);
        }
    } else if (extension == "csv") {
        // Поддержка CSV по RFC 4180: кавычки, экранированные кавычки и переносы
        // строк внутри полей; также распознаём экспорт с разделителем «;».
        const QChar delimiter = text.section('\n', 0, 0).count(';') > text.section('\n', 0, 0).count(',') ? ';' : ',';
        QList<QStringList> rows;
        QStringList row;
        QString field;
        bool quoted = false;
        for (qsizetype i = 0; i < text.size(); ++i) {
            QChar c = text.at(i);
            if (c == '"') {
                if (quoted && i + 1 < text.size() && text.at(i + 1) == '"') {
                    field += '"';
                    ++i;
                } else
                    quoted = !quoted;
            } else if (!quoted && (c == delimiter || c == '\n')) {
                row << field.trimmed();
                field.clear();
                if (c == '\n') {
                    rows << row;
                    row.clear();
                }
            } else if (c != '\r')
                field += c;
        }
        if (quoted) {
            *error = "В CSV не закрыта кавычка.";
            return {};
        }
        if (!field.isEmpty() || !row.isEmpty()) {
            row << field.trimmed();
            rows << row;
        }
        if (!rows.isEmpty()) {
            auto header = rows.takeFirst();
            for (auto& v : header)
                v = v.toLower().trimmed();
            auto col = [&](const QStringList& names) {
                for (const auto& n : names) {
                    int i = header.indexOf(n);
                    if (i >= 0)
                        return i;
                }
                return -1;
            };
            int title = col({"title", "track", "track name", "название"}),
                artist = col({"artist", "artists", "artist name(s)", "исполнитель"}),
                album = col({"album", "album name", "альбом"}), url = col({"url", "source_url", "track uri", "ссылка"});
            if (title < 0) {
                *error = "CSV должен содержать колонку title (или название), artist — необязательна.";
                return {};
            }
            for (const auto& r : rows) {
                Track t;
                t.title = r.value(title);
                t.artist = r.value(artist);
                t.album = r.value(album);
                t.sourceUrl = r.value(url);
                t.source = source;
                if (!t.title.isEmpty())
                    tracks.append(t);
            }
        }
    } else {
        // Обычный текст: каждая строка вида «исполнитель — название».
        const QRegularExpression split("\\s+[—–-]\\s+");
        for (QString line : text.split('\n')) {
            line = line.trimmed();
            if (line.isEmpty() || line.startsWith('#'))
                continue;
            Track t;
            t.source = source;
            auto match = split.match(line);
            if (match.hasMatch()) {
                t.artist = line.left(match.capturedStart()).trimmed();
                t.title = line.mid(match.capturedEnd()).trimmed();
            } else
                t.title = line;
            if (!t.title.isEmpty())
                tracks.append(t);
        }
    }
    if (tracks.size() > 10000) {
        *error = "За один импорт можно добавить до 10 000 треков.";
        return {};
    }
    if (tracks.isEmpty())
        *error = "В файле не найдено треков.";
    return tracks;
}
// Сохраняет треки в новый плейлист одной транзакцией и формирует превью для UI.
void LibraryExchange::saveList(const QVector<Track>& tracks, const QString& name)
{
    auto& repo = m_library->repository();
    if (!repo.transaction()) {
        finish(repo.error());
        return;
    }
    // Older DOM imports used /album/.../track/... URLs. Reuse their IDs and files
    // when the complete API import returns the canonical /track/... identity.
    QHash<QString, Track> yandexExisting;
    for (const auto& track : repo.tracks()) {
        const auto key = ServicePayload::canonicalYandexTrackUrl(QUrl(track.sourceUrl));
        if (!key.isEmpty() && (!yandexExisting.contains(key) || !track.filePath.isEmpty()))
            yandexExisting[key] = track;
    }
    int playlist = repo.createPlaylist(name.isEmpty() ? QStringLiteral("Импортированный плейлист") : name);
    bool ok = playlist > 0;
    QVariantList preview;
    for (const auto& track : tracks) {
        if (!ok)
            break;
        const auto canonical = ServicePayload::canonicalYandexTrackUrl(QUrl(track.sourceUrl));
        int id = !canonical.isEmpty() && yandexExisting.contains(canonical)
                     ? yandexExisting.value(canonical).id : repo.saveTrack(track);
        ok = id && repo.addToPlaylist(playlist, id);
        auto item = track.webData;
        item.insert("trackId", id);
        item.insert("title", track.title);
        item.insert("artist", track.artist);
        item.insert("durationMs", track.duration);
        item.insert("url", track.sourceUrl);
        item.insert("mediaUrl", track.mediaUrl);
        item.insert("coverUrl", track.coverUrl);
        item.insert("downloaded", QFileInfo::exists(repo.track(id).filePath));
        preview.append(item);
    }
    if (!ok || !repo.commit()) {
        repo.rollback();
        finish("Импорт отменён: " + repo.error());
        return;
    }
    m_importedTracks = preview;
    // Ставим в очередь скачивание обложек для импортированных треков.
    for (const auto& entry : preview) {
        const auto item = entry.toMap();
        m_library->cacheCover(item.value("trackId").toInt(), QUrl(item.value("coverUrl").toString()));
    }
    const bool download = m_downloadAfterImport;
    m_downloadAfterImport = false;
    m_library->refresh();
    finish(download ? QString("Перенесено %1 треков. Скачиваем аудио в плеер…").arg(tracks.size())
                    : QString("В коллекции %1 треков. Выбери аудио для загрузки ниже.").arg(tracks.size()),
           playlist);
    if (download)
        emit audioImportRequested(preview);
}
// Импортирует «мою музыку» сервиса через существующую сессию браузера.
void LibraryExchange::importMyMusic(const QString& service, const QString& profileId)
{
    if (m_busy || !m_webLogin)
        return;
    if (m_webLogin->busy()) {
        finish("Дождись завершения текущего импорта.");
        return;
    }
    const QString slug = WebLoginManager::normalizeService(service);
    if (slug.isEmpty()) {
        finish("Выбери Яндекс Музыку или VK.");
        return;
    }
    m_busy = true;
    m_nativeImport = true;
    m_downloadAfterImport = true;
    m_importedTracks.clear();
    m_status = "Читаем существующий вход в браузере…";
    emit changed();
    m_webLogin->collectFromBrowser(slug, {}, profileId);
}
// Перечитывает названия VK-треков и чинит испорченные, не трогая файлы.
void LibraryExchange::repairVkMetadata()
{
    if (m_busy || !m_webLogin)
        return;
    if (m_webLogin->busy()) {
        finish("VK занят другой операцией. Повтори обновление названий чуть позже.");
        return;
    }
    m_metadataOnly = true;
    m_nativeImport = true;
    m_downloadAfterImport = false;
    m_busy = true;
    m_status = "Читаем названия из VK заново…";
    emit changed();
    m_webLogin->collectFromBrowser("VK", {}, QSettings().value("weblogin-local/vk").toString());
}
// Импортирует по ссылке: если хост — известный сервис, читаем через браузер,
// иначе пробуем обработать как обычный URL.
void LibraryExchange::importFromBrowser(const QString& raw, const QString& profileId)
{
    if (m_busy || !m_webLogin)
        return;
    const QUrl url(raw.trimmed());
    const QString slug = WebLoginManager::serviceForHost(url.host());
    if (slug.isEmpty()) {
        importUrl(raw);
        return;
    }
    if (m_webLogin->busy()) {
        finish("Дождись завершения текущего импорта.");
        return;
    }
    m_busy = true;
    m_nativeImport = true;
    m_downloadAfterImport = true;
    m_importedTracks.clear();
    m_status = "Читаем существующий вход в браузере…";
    emit changed();
    m_webLogin->collectFromBrowser(slug, url, profileId);
}
// Импортирует локальный файл: переносимый архив или текстовый список.
void LibraryExchange::importList(const QUrl& url, const QString& source)
{
    if (m_busy || !url.isLocalFile())
        return;
    QFile file(url.toLocalFile());
    if (!file.open(QIODevice::ReadOnly) || file.size() > maxBytes) {
        finish("Не удалось открыть файл или он больше 20 МБ.");
        return;
    }
    QByteArray bytes = file.readAll();
    auto manifest = QJsonDocument::fromJson(bytes).object();
    m_busy = true;
    m_status = "Читаем плейлист…";
    emit changed();
    // Переносимый архив имеет свою версию манифеста.
    if (manifest.value("auroraVersion").toInt() == 1) {
        importPortable(url, manifest);
        return;
    }
    QString error;
    auto tracks = parseText(bytes, QFileInfo(file).suffix().toLower(), source, &error);
    if (!error.isEmpty()) {
        finish(error);
        return;
    }
    saveList(tracks, QFileInfo(file).completeBaseName());
}
// Импортирует по ссылке, выбирая способ по хосту: браузер, API Яндекса,
// yt-dlp для YouTube или VkProvider для VK.
void LibraryExchange::importUrl(const QString& raw)
{
    if (m_busy)
        return;
    const QUrl url(raw.trimmed());
    const QString host = url.host().toLower();
    if (url.scheme() != "https" && url.scheme() != "http") {
        finish("Вставьте ссылку http или https.");
        return;
    }
    const QString nativeService = WebLoginManager::serviceForHost(host);
    if (!nativeService.isEmpty() && m_webLogin) {
        if (m_webLogin->busy()) {
            finish("Заверши текущую операцию в окне сервиса.");
            return;
        }
        m_busy = true;
        m_nativeImport = true;
        m_importedTracks.clear();
        m_status = "Открываем сервис…";
        emit changed();
        m_webLogin->collectPlaylist(url);
        return;
    }
    if (QStringList{"music.yandex.ru", "music.yandex.com", "music.yandex.kz", "music.yandex.by", "music.yandex.uz"}
            .contains(host)) {
        // Есть токен — идём через авторизованный API, иначе через публичную ссылку.
        const QString token = m_oauth ? m_oauth->yandexToken() : QString();
        if (!token.isEmpty()) {
            fetchYandexAuthed(url, token, false);
            return;
        }
        importYandexLink(url);
        return;
    }
    if (host == "youtu.be" || host == "youtube.com" || host.endsWith(".youtube.com")) {
        QString program = m_dependencies ? m_dependencies->ytDlpExecutable() : QString();
        if (program.isEmpty()) {
            finish("Для YouTube установите yt-dlp в разделе загрузчиков ниже.");
            return;
        }
        m_busy = true;
        m_status = "Получаем список YouTube…";
        emit changed();
        auto* process = new QProcess(this);
        m_process = process;
        // Ограничиваем время работы yt-dlp одной минутой.
        auto* timer = new QTimer(process);
        timer->setSingleShot(true);
        timer->start(60000);
        connect(timer, &QTimer::timeout, process, &QProcess::kill);
        connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError e) {
            if (e == QProcess::FailedToStart) {
                m_process = nullptr;
                process->deleteLater();
                finish("Не удалось запустить yt-dlp.");
            }
        });
        connect(process, &QProcess::readyReadStandardOutput, this, [process] {
            if (process->bytesAvailable() > maxBytes)
                process->kill();
        });
        connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
                [this, process](int code, QProcess::ExitStatus exit) {
                    auto bytes = process->readAllStandardOutput();
                    process->deleteLater();
                    m_process = nullptr;
                    if (code != 0 || exit != QProcess::NormalExit || bytes.size() > maxBytes) {
                        finish("YouTube не отдал список. Проверьте доступность ссылки и версию yt-dlp.");
                        return;
                    }
                    QString error;
                    auto tracks = parseText(bytes, "json", "YouTube", &error);
                    if (!error.isEmpty()) {
                        finish(error);
                        return;
                    }
                    saveList(tracks, QJsonDocument::fromJson(bytes).object().value("title").toString("YouTube"));
                });
        process->start(program,
                       {"--ignore-config", "--flat-playlist", "--dump-single-json", "--skip-download", "--no-warnings",
                        "--socket-timeout", "20", "--playlist-end", "10000", "--", url.toString()});
        return;
    }
    QString normalized = host.startsWith("www.") ? host.mid(4) : host;
    if (normalized == "vk.com" || normalized == "vk.ru" || normalized.endsWith(".vk.com") ||
        normalized.endsWith(".vk.ru")) {
        // Для VK нужен вход: либо токен OAuth, либо сессия браузера.
        if ((!m_oauth || m_oauth->vkToken().isEmpty()) && (!m_webLogin || !m_webLogin->hasSession("vk"))) {
            finish("Для VK нужен вход: нажмите «Войти в VK» на странице импорта (вход через браузер) или подключите "
                   "токен.");
            return;
        }
        m_busy = true;
        m_status = "Получаем список VK…";
        emit changed();
        m_vk->fetch(url.toString());
        return;
    }
    finish("Поддерживаются ссылки на плейлисты Яндекс Музыки, YouTube и VK (vk.com/audios…, /music/playlist/…, "
           "/music/album/…). Другие сервисы импортируйте TXT/CSV/JSON файлом.");
}
// Возвращает куки браузерной сессии для api Яндекс Музыки (или пустой заголовок).
QByteArray LibraryExchange::yandexCookieHeader() const
{
    if (!m_webLogin || !m_webLogin->hasSession("yandex"))
        return {};
    const QString header = m_webLogin->cookieHeader("yandex", QUrl("https://music.yandex.ru"));
    return header.isEmpty() ? QByteArray() : header.toUtf8();
}
// Получает публичный плейлист Яндекс Музыки по ссылке (без токена).
void LibraryExchange::importYandexLink(const QUrl& url)
{
    const QStringList parts = url.path().split('/', Qt::SkipEmptyParts);
    QUrl endpoint;
    if (parts.size() == 2 && parts[0] == "playlists")
        endpoint =
            QUrl("https://api.music.yandex.ru/playlist/" + QString::fromLatin1(QUrl::toPercentEncoding(parts[1])));
    else if (parts.size() == 4 && parts[0] == "users" && parts[2] == "playlists") {
        endpoint = QUrl("https://music.yandex.ru/handlers/playlist.jsx");
        QUrlQuery query;
        query.addQueryItem("owner", parts[1]);
        query.addQueryItem("kinds", parts[3]);
        endpoint.setQuery(query);
    }
    if (endpoint.isEmpty()) {
        finish("Нужна публичная ссылка Яндекс Музыки вида /playlists/… или /users/…/playlists/…");
        return;
    }
    m_busy = true;
    m_status = "Получаем список Яндекс Музыки…";
    emit changed();
    QNetworkRequest request(endpoint);
    request.setTransferTimeout(30000);
    request.setRawHeader("User-Agent", "AuroraPlayer/4.0");
    const QByteArray cookies = yandexCookieHeader();
    if (!cookies.isEmpty())
        request.setRawHeader("Cookie", cookies);
    auto* reply = m_network.get(request);
    connect(reply, &QNetworkReply::downloadProgress, this, [reply](qint64 received, qint64) {
        if (received > maxBytes)
            reply->abort();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        const auto bytes = reply->readAll();
        const auto error = reply->error();
        reply->deleteLater();
        if (error != QNetworkReply::NoError) {
            finish("Сервис не отдал плейлист. Проверьте, что он публичный, или импортируйте TXT/CSV/JSON экспорт.");
            return;
        }
        auto root = QJsonDocument::fromJson(bytes).object();
        auto playlist = root.value("result").toObject();
        if (playlist.isEmpty())
            playlist = root.value("playlist").toObject();
        completeYandexPlaylist(playlist);
    });
}
// Получает плейлист Яндекс Музыки через авторизованный API с OAuth-токеном.
void LibraryExchange::fetchYandexAuthed(const QUrl& url, const QString& token, bool retried)
{
    const QStringList parts = url.path().split('/', Qt::SkipEmptyParts);
    QUrl endpoint;
    if (parts.size() == 4 && parts[0] == "users" && parts[2] == "playlists")
        endpoint = QUrl("https://api.music.yandex.ru/users/" + parts[1] + "/playlists/" + parts[3]);
    else if (parts.size() == 2 && parts[0] == "playlists")
        endpoint =
            QUrl("https://api.music.yandex.ru/playlists/" + QString::fromLatin1(QUrl::toPercentEncoding(parts[1])));
    if (endpoint.isEmpty() || token.isEmpty()) {
        importYandexLink(url);
        return;
    }
    m_busy = true;
    m_status = "Получаем список Яндекс Музыки…";
    emit changed();
    QNetworkRequest request(endpoint);
    request.setTransferTimeout(30000);
    request.setRawHeader("User-Agent", "AuroraPlayer/4.0");
    request.setRawHeader("Authorization", "OAuth " + token.toUtf8());
    const QByteArray cookies = yandexCookieHeader();
    if (!cookies.isEmpty())
        request.setRawHeader("Cookie", cookies);
    auto* reply = m_network.get(request);
    connect(reply, &QNetworkReply::downloadProgress, this, [reply](qint64 received, qint64) {
        if (received > maxBytes)
            reply->abort();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, url, token, retried] {
        const auto bytes = reply->readAll();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const auto netError = reply->error();
        reply->deleteLater();
        // Токен протух — пробуем один раз обновить его и повторить запрос.
        if ((status == 401 || status == 403) && !retried && m_oauth && m_oauth->refreshYandexToken()) {
            const QString fresh = m_oauth->yandexToken();
            if (!fresh.isEmpty()) {
                fetchYandexAuthed(url, fresh, true);
                return;
            }
        }
        if (netError != QNetworkReply::NoError) {
            importYandexLink(url);
            return;
        }
        auto playlist = QJsonDocument::fromJson(bytes).object().value("result").toObject();
        if (playlist.isEmpty()) {
            importYandexLink(url);
            return;
        }
        const auto entries = playlist.value("tracks").toArray();
        const auto trackIds = playlist.value("trackIds").toArray();
        const int expected = playlist.value("trackCount").toInt(trackIds.size());
        QStringList missing;
        // Сравниваем полученные треки с полным списком ID и находим недостающие.
        if (expected > entries.size()) {
            QSet<QString> have;
            for (const auto& value : entries) {
                const auto object = value.toObject();
                const auto track = object.value("track").isObject() ? object.value("track").toObject() : object;
                have.insert(track.value("id").toVariant().toString());
            }
            for (const auto& id : trackIds) {
                const QString key = id.toVariant().toString().section(':', 0, 0);
                if (!have.contains(key))
                    missing.append(key);
            }
        }
        if (!missing.isEmpty()) {
            fetchYandexMissing(playlist, missing, token);
            return;
        }
        completeYandexPlaylist(playlist);
    });
}
// Догружает недостающие треки Яндекс-плейлиста отдельным запросом к /tracks.
void LibraryExchange::fetchYandexMissing(const QJsonObject& playlist, const QStringList& missing, const QString& token)
{
    QUrl endpoint("https://api.music.yandex.ru/tracks");
    QUrlQuery query;
    // API принимает не более 500 ID за раз.
    query.addQueryItem("track-ids", missing.mid(0, 500).join(','));
    query.addQueryItem("with-positions", "false");
    endpoint.setQuery(query);
    m_status = QString("Получено %1 из %2 треков…")
                   .arg(playlist.value("tracks").toArray().size())
                   .arg(playlist.value("trackCount").toInt());
    emit changed();
    QNetworkRequest request(endpoint);
    request.setTransferTimeout(30000);
    request.setRawHeader("User-Agent", "AuroraPlayer/4.0");
    request.setRawHeader("Authorization", "OAuth " + token.toUtf8());
    const QByteArray cookies = yandexCookieHeader();
    if (!cookies.isEmpty())
        request.setRawHeader("Cookie", cookies);
    auto* reply = m_network.get(request);
    connect(reply, &QNetworkReply::downloadProgress, this, [reply](qint64 received, qint64) {
        if (received > maxBytes)
            reply->abort();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, playlist]() mutable {
        const auto bytes = reply->readAll();
        const bool ok = reply->error() == QNetworkReply::NoError;
        reply->deleteLater();
        if (ok) {
            auto document = QJsonDocument::fromJson(bytes);
            auto batch = document.isArray() ? document.array() : document.object().value("result").toArray();
            QJsonArray entries = playlist.value("tracks").toArray();
            for (const auto& value : batch)
                entries.append(QJsonObject{{"track", value.toObject()}});
            playlist["tracks"] = entries;
        }
        completeYandexPlaylist(playlist);
    });
}
// Собирает полный плейлист Яндекса: догружает недостающие треки и сохраняет.
void LibraryExchange::completeYandexPlaylist(QJsonObject playlist)
{
    auto entries = playlist.value("tracks").toArray();
    const auto trackIds = playlist.value("trackIds").toArray();
    int expected = playlist.value("trackCount").toInt(trackIds.size());
    if (expected > 10000 || entries.size() > 10000) {
        finish("За один импорт можно перенести до 10 000 треков.");
        return;
    }
    QHash<QString, QJsonObject> byId;
    for (const auto& entry : entries) {
        auto obj = entry.toObject();
        auto track = obj.value("track").isObject() ? obj.value("track").toObject() : obj;
        const auto id = track.value("id").toVariant().toString();
        if (!id.isEmpty())
            byId[id] = obj;
    }
    QStringList missing;
    for (const auto& id : trackIds) {
        const QString key = id.toVariant().toString().section(':', 0, 0);
        if (!byId.contains(key))
            missing.append(key);
    }
    if (!missing.isEmpty()) {
        m_status = QString("Получено %1 из %2 треков…").arg(entries.size()).arg(expected);
        emit changed();
        // Недостающие треки догружаем через handlers/track-entries.jsx.
        QUrl endpoint("https://music.yandex.ru/handlers/track-entries.jsx");
        QUrlQuery query;
        query.addQueryItem("entries", missing.mid(0, 250).join(','));
        query.addQueryItem("lang", "ru");
        query.addQueryItem("external-domain", "music.yandex.ru");
        query.addQueryItem("overembed", "false");
        query.addQueryItem("strict", "true");
        endpoint.setQuery(query);
        QNetworkRequest request(endpoint);
        request.setTransferTimeout(30000);
        request.setRawHeader("User-Agent", "AuroraPlayer/4.0");
        const QByteArray cookies = yandexCookieHeader();
        if (!cookies.isEmpty())
            request.setRawHeader("Cookie", cookies);
        auto* reply = m_network.get(request);
        connect(reply, &QNetworkReply::downloadProgress, this, [reply](qint64 received, qint64) {
            if (received > maxBytes)
                reply->abort();
        });
        connect(reply, &QNetworkReply::finished, this, [this, reply, playlist, entries, byId]() mutable {
            auto doc = QJsonDocument::fromJson(reply->readAll());
            const bool ok = reply->error() == QNetworkReply::NoError;
            reply->deleteLater();
            QJsonArray batch = doc.isArray() ? doc.array() : doc.object().value("tracks").toArray();
            int added = 0;
            for (const auto& entry : batch) {
                auto obj = entry.toObject();
                auto track = obj.value("track").isObject() ? obj.value("track").toObject() : obj;
                const auto id = track.value("id").toVariant().toString();
                if (!id.isEmpty() && !byId.contains(id)) {
                    entries.append(entry);
                    byId[id] = obj;
                    ++added;
                }
            }
            if (!ok || !added) {
                finish(
                    "Сервис вернул неполный плейлист. Импортируйте TXT/CSV/JSON экспорт, чтобы сохранить все треки.");
                return;
            }
            playlist["tracks"] = entries;
            completeYandexPlaylist(playlist);
        });
        return;
    }
    if (!trackIds.isEmpty()) {
        // Восстанавливаем порядок треков по списку ID.
        QJsonArray ordered;
        for (const auto& id : trackIds)
            ordered.append(byId.value(id.toVariant().toString().section(':', 0, 0)));
        playlist["tracks"] = ordered;
    }
    QString error;
    auto tracks = parseText(QJsonDocument(playlist).toJson(), "json", "Яндекс Музыка", &error);
    if (!error.isEmpty()) {
        finish(error + " Можно импортировать текстовый экспорт.");
        return;
    }
    if (expected > tracks.size()) {
        finish(QString("Сервис вернул %1 из %2 треков. Импортируйте полный TXT/CSV/JSON экспорт.")
                   .arg(tracks.size())
                   .arg(expected));
        return;
    }
    saveList(tracks, playlist.value("title").toString("Яндекс Музыка"));
}
// Сохраняет простой текстовый список треков («исполнитель — название»).
void LibraryExchange::exportText(const QUrl& url, int playlist)
{
    if (m_busy || !url.isLocalFile())
        return;
    QSaveFile file(url.toLocalFile());
    if (!file.open(QIODevice::WriteOnly)) {
        finish(file.errorString());
        return;
    }
    QByteArray output;
    for (const auto& t : m_library->repository().tracks({}, playlist))
        output += (t.artist + " — " + t.title + "\n").toUtf8();
    if (file.write(output) != output.size() || !file.commit()) {
        finish(file.errorString());
        return;
    }
    finish("Список треков сохранён.");
}
// Собирает переносимую медиатеку: копирует аудио и обложки + манифест и m3u.
void LibraryExchange::exportPortable(const QUrl& folder, int playlist)
{
    if (m_busy || !folder.isLocalFile())
        return;
    const auto tracks = m_library->repository().tracks({}, playlist);
    if (tracks.isEmpty()) {
        finish("В выбранной коллекции пока нет треков.");
        return;
    }
    const auto lists = m_library->repository().playlists();
    QJsonArray playlists;
    for (const auto& v : lists) {
        auto p = v.toMap();
        int id = p.value("id").toInt();
        if (playlist > 0 && playlist != id)
            continue;
        QJsonArray ids;
        for (const auto& t : m_library->repository().tracks({}, id)) {
            for (const auto& included : tracks)
                if (included.id == t.id) {
                    ids.append(t.id);
                    break;
                }
        }
        playlists.append(QJsonObject{{"name", p.value("name").toString()}, {"tracks", ids}});
    }
    const QString target =
        QDir(folder.toLocalFile()).filePath("Aurora-" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8));
    m_busy = true;
    m_status = "Собираем переносимую медиатеку…";
    emit changed();
    // Копирование файлов выполняем в фоновом потоке, чтобы не блокировать UI.
    auto* watcher = new QFutureWatcher<QString>(this);
    connect(watcher, &QFutureWatcher<QString>::finished, this, [this, watcher] {
        QString result = watcher->result();
        watcher->deleteLater();
        finish(result);
    });
    watcher->setFuture(QtConcurrent::run([tracks, playlists, target] {
        QDir root(target);
        if (!root.mkpath("audio") || !root.mkpath("covers"))
            return QString("Не удалось создать папку экспорта.");
        QJsonArray entries;
        QByteArray m3u = "#EXTM3U\n";
        int missing = 0;
        // При любой ошибке удаляем собранную папку и возвращаем сообщение.
        auto fail = [&](const QString& message) {
            root.removeRecursively();
            return message;
        };
        for (const auto& t : tracks) {
            QString audio, cover;
            if (!t.filePath.isEmpty() && QFileInfo::exists(t.filePath)) {
                audio = "audio/" + QString::number(t.id) + "." + QFileInfo(t.filePath).suffix();
                if (!QFile::copy(t.filePath, root.filePath(audio)))
                    return fail("Не удалось скопировать " + t.title + ". Проверьте свободное место.");
                QString label = t.artist + " — " + t.title;
                // В m3u-записи не должно быть переносов строк.
                label.replace('\n', ' ');
                label.replace('\r', ' ');
                m3u += ("#EXTINF:" + QString::number(t.duration / 1000) + "," + label + "\n" + audio + "\n").toUtf8();
            } else
                ++missing;
            if (!t.coverPath.isEmpty() && QFileInfo::exists(t.coverPath)) {
                cover = "covers/" + QString::number(t.id) + ".jpg";
                if (!QFile::copy(t.coverPath, root.filePath(cover)))
                    return fail("Не удалось скопировать обложку.");
            }
            entries.append(QJsonObject{{"id", t.id},
                                       {"title", t.title},
                                       {"artist", t.artist},
                                       {"album", t.album},
                                       {"source", t.source},
                                       {"url", t.sourceUrl},
                                       {"file", audio},
                                       {"cover", cover},
                                       {"format", t.format},
                                       {"durationMs", t.duration},
                                       {"bitrate", t.bitrate},
                                       {"rating", t.rating}});
        }
        QSaveFile manifest(root.filePath("aurora-library.json")), playlist(root.filePath("library.m3u8"));
        QByteArray bytes =
            QJsonDocument(QJsonObject{{"auroraVersion", 1}, {"tracks", entries}, {"playlists", playlists}}).toJson();
        if (!manifest.open(QIODevice::WriteOnly) || manifest.write(bytes) != bytes.size() || !manifest.commit() ||
            !playlist.open(QIODevice::WriteOnly) || playlist.write(m3u) != m3u.size() || !playlist.commit())
            return fail("Не удалось записать экспорт.");
        return QString("Сохранено в %1 · аудиофайлов: %2 · записей без файла: %3")
            .arg(target)
            .arg(tracks.size() - missing)
            .arg(missing);
    }));
}
// Импортирует переносимый архив: проверяет манифест, копирует файлы и треки.
void LibraryExchange::importPortable(const QUrl& url, const QJsonObject& manifest)
{
    QDir root(QFileInfo(url.toLocalFile()).absolutePath());
    auto entries = manifest.value("tracks").toArray();
    if (entries.isEmpty() || entries.size() > 10000) {
        finish("Некорректная или слишком большая медиатека.");
        return;
    }
    // Сначала проверяем целостность всех записей и путей, не меняя данные.
    QSet<int> seen;
    for (const auto& v : entries) {
        auto o = v.toObject();
        int id = o.value("id").toInt();
        if (id <= 0 || seen.contains(id) || o.value("title").toString().isEmpty()) {
            finish("Некорректные записи медиатеки.");
            return;
        }
        seen.insert(id);
        for (const QString& key : QStringList{"file", "cover"})
            if (!o.value(key).toString().isEmpty() && bundleFile(root, o.value(key).toString()).isEmpty()) {
                finish("В экспорте отсутствует файл или путь выходит за пределы его папки.");
                return;
            }
    }
    for (const auto& v : manifest.value("playlists").toArray())
        for (const auto& id : v.toObject().value("tracks").toArray())
            if (!seen.contains(id.toInt())) {
                finish("В плейлисте есть неизвестный ID трека.");
                return;
            }
    QDir destination(m_settings->libraryFolder());
    destination.mkpath(".");
    QStringList created;
    QHash<int, int> mapping;
    auto& repo = m_library->repository();
    if (!repo.transaction()) {
        finish(repo.error());
        return;
    }
    bool ok = true;
    for (const auto& v : entries) {
        auto o = v.toObject();
        Track t = jsonTrack(o, o.value("source").toString("Local"));
        t.rating = qBound(0, o.value("rating").toInt(), 5);
        t.format = o.value("format").toString();
        t.bitrate = o.value("bitrate").toInt();
        for (const QString& key : QStringList{"file", "cover"}) {
            QString relative = o.value(key).toString();
            if (relative.isEmpty())
                continue;
            QString source = bundleFile(root, relative);
            QFile file(source);
            if (!file.open(QIODevice::ReadOnly)) {
                ok = false;
                break;
            }
            // Имя файла в медиатеке — хэш SHA-256 содержимого, чтобы избежать дублей.
            QCryptographicHash hash(QCryptographicHash::Sha256);
            if (!hash.addData(&file)) {
                ok = false;
                break;
            }
            QString target =
                destination.filePath(QString::fromLatin1(hash.result().toHex()) + "." + QFileInfo(source).suffix());
            if (!QFileInfo::exists(target)) {
                if (!QFile::copy(source, target)) {
                    ok = false;
                    break;
                }
                created << target;
            }
            if (key == "file")
                t.filePath = target;
            else
                t.coverPath = target;
        }
        if (!ok)
            break;
        int id = repo.saveTrack(t);
        if (!id) {
            ok = false;
            break;
        }
        mapping[o.value("id").toInt()] = id;
        if (!repo.setRating(id, t.rating)) {
            ok = false;
            break;
        }
    }
    int lastPlaylist = 0;
    if (ok)
        for (const auto& v : manifest.value("playlists").toArray()) {
            auto o = v.toObject();
            int id = repo.createPlaylist(o.value("name").toString());
            lastPlaylist = id;
            if (!id) {
                ok = false;
                break;
            }
            for (const auto& track : o.value("tracks").toArray())
                if (!repo.addToPlaylist(id, mapping.value(track.toInt()))) {
                    ok = false;
                    break;
                }
            if (!ok)
                break;
        }
    if (!ok || !repo.commit()) {
        repo.rollback();
        // Откатываем уже скопированные файлы при неудаче.
        for (const auto& file : created)
            QFile::remove(file);
        finish("Перенос не завершён: " + repo.error());
        return;
    }
    m_library->refresh();
    finish(QString("Медиатека восстановлена: %1 треков, плейлисты и оценки сохранены.").arg(mapping.size()),
           lastPlaylist);
}

// Прерывает текущую операцию: браузерный импорт или внешний процесс.
void LibraryExchange::cancel()
{
    if (m_nativeImport && m_webLogin)
        m_webLogin->cancel();
    else if (m_process)
        m_process->kill();
}
