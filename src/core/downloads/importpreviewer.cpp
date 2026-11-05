#include "importpreviewer.h"

#include "auth/browserauthmanager.h"
#include "dependencymanager.h"
#include "downloadmanager.h"
#include "errormapper.h"
#include "settingsstore.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QProcess>
#include <QTimer>
#include <QStandardPaths>
#include <QUrl>
#include <QRegularExpression>

#include <limits>
#include <utility>

// Реализация предпросмотра импорта: запуск yt-dlp для метаданных, парсинг ответа и оценка размера.
struct ImportPreviewer::Item
{
    QString title;
    QString artist;
    QString url;
    qint64 duration = 0;
    qint64 size = -1;
    bool selected = true;
};

namespace
{

// Оценка размера файла по длительности и битрейту (128/192/320 кбит/с → байты).
qint64 estimateBytes(qint64 durationMs, const QString& quality)
{
    if (durationMs <= 0) {
        return -1;
    }
    const qint64 bitrate = quality == QStringLiteral("compact")    ? 128
                           : quality == QStringLiteral("balanced") ? 192
                                                                   : 320;
    const long double estimate = static_cast<long double>(durationMs) * bitrate / 8.0L;
    return estimate >= std::numeric_limits<qint64>::max() ? std::numeric_limits<qint64>::max()
                                                          : static_cast<qint64>(estimate);
}

qint64 jsonInteger(const QJsonObject& object, const QString& key)
{
    const QJsonValue value = object.value(key);
    if (!value.isDouble()) {
        return -1;
    }
    const double number = value.toDouble(-1.0);
    return number >= 0.0 ? static_cast<qint64>(number) : -1;
}

} // namespace

ImportPreviewer::ImportPreviewer(SettingsStore* settings, DependencyManager* dependencies, BrowserAuthManager* auth,
                                 DownloadManager* downloads, QObject* parent)
    : QAbstractListModel(parent), m_settings(settings), m_dependencies(dependencies), m_auth(auth),
      m_downloads(downloads), m_statusText(tr("Введите ссылку для предпросмотра"))
{}

ImportPreviewer::~ImportPreviewer()
{
    if (m_process) {
        disconnect(m_process, nullptr, this, nullptr);
        m_process->kill();
        m_process->waitForFinished(1000);
        delete m_process;
        m_process = nullptr;
    }
}

int ImportPreviewer::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant ImportPreviewer::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size()) {
        return {};
    }
    const Item& item = m_items.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return item.title;
    case ArtistRole:
        return item.artist;
    case UrlRole:
        return item.url;
    case DurationRole:
        return item.duration;
    case SizeRole:
        return item.size;
    case SelectedRole:
        return item.selected;
    default:
        return {};
    }
}

QHash<int, QByteArray> ImportPreviewer::roleNames() const
{
    return {{TitleRole, "title"},       {ArtistRole, "artist"}, {UrlRole, "url"},
            {DurationRole, "duration"}, {SizeRole, "size"},     {SelectedRole, "selected"}};
}

void ImportPreviewer::inspect(const QString& url, const QString& quality, bool authorized)
{
    startPreview(url, quality, authorized, false);
}
void ImportPreviewer::searchYoutube(const QString& query, const QString& quality)
{
    if (query.trimmed().isEmpty() || query.size() > 200) {
        setLastError(tr("Введите исполнителя или название, до 200 символов."));
        return;
    }
    startPreview("ytsearch10:" + query.trimmed(), quality, false, true);
}
void ImportPreviewer::startPreview(const QString& url, const QString& quality, bool authorized, bool searching)
{
    if (m_busy) {
        setLastError(tr("Дождитесь завершения текущего предпросмотра."));
        return;
    }
    const QString normalizedQuality = quality.trimmed().toLower();
    if (normalizedQuality != QStringLiteral("original") && normalizedQuality != QStringLiteral("balanced") &&
        normalizedQuality != QStringLiteral("compact")) {
        setLastError(tr("Некорректное качество. Используйте original, balanced или compact."));
        return;
    }
    const QString source = url.trimmed();
    const QString service = searching     ? QStringLiteral("YouTube")
                            : m_downloads ? m_downloads->serviceForUrl(source)
                                          : QString();
    if (service.isEmpty()) {
        setLastError(tr("Введите корректный HTTP(S) URL YouTube, Яндекс Музыки, VK или Spotify."));
        return;
    }

    m_quality = normalizedQuality;
    m_authorized = authorized;
    m_searching = searching;
    setSourceUrl(searching ? "https://www.youtube.com/results?search_query=" +
                                 QString::fromLatin1(QUrl::toPercentEncoding(source.mid(11)))
                           : source);
    setLastError({});
    beginResetModel();
    m_items.clear();
    endResetModel();
    updateSummary();

    if (service == QStringLiteral("Spotify")) {
        inspectSpotify(source);
        return;
    }

    const QString program = m_dependencies ? m_dependencies->ytDlpExecutable() : QString();
    if (program.isEmpty()) {
        finishError(tr("yt-dlp недоступен. Установите загрузчики через менеджер зависимостей."));
        return;
    }

    m_stdout.clear();
    m_stderr.clear();
    m_stopReason.clear();
    setBusy(true);
    setStatusText(tr("Получение метаданных и списка треков…"));

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);
    // Общий таймаут 90 секунд на получение метаданных.
    auto* timeout = new QTimer(m_process);
    timeout->setSingleShot(true);
    timeout->start(90000);
    connect(timeout, &QTimer::timeout, this, [this] {
        m_stopReason = tr("Сервис не ответил за 90 секунд. Проверь доступность ссылки и соединение.");
        if (m_process)
            m_process->kill();
    });
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this] {
        if (m_process) {
            m_stdout.append(m_process->readAllStandardOutput());
            // Огромные списки обрезаем, чтобы не раздувать память.
            if (m_stdout.size() > 20 * 1024 * 1024) {
                m_stopReason = tr("Список слишком большой. Открой отдельный плейлист.");
                m_process->kill();
            }
        }
    });
    connect(m_process, &QProcess::readyReadStandardError, this, [this] {
        if (m_process) {
            m_stderr = (m_stderr + QString::fromLocal8Bit(m_process->readAllStandardError())).right(8192);
        }
    });
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart && m_process) {
            const QString details = m_process->errorString();
            finishError(tr("Не удалось запустить yt-dlp: %1").arg(details));
        }
    });
    connect(m_process, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (!m_process) {
            return;
        }
        m_stdout.append(m_process->readAllStandardOutput());
        m_stderr = (m_stderr + QString::fromLocal8Bit(m_process->readAllStandardError())).right(8192);
        m_process->deleteLater();
        m_process = nullptr;
        if (!m_stopReason.isEmpty()) {
            finishError(m_stopReason);
            return;
        }
        if (exitStatus != QProcess::NormalExit || exitCode != 0) {
            QString message =
                exitStatus == QProcess::CrashExit
                    ? tr("Процесс yt-dlp остановился (сигнал %1). Проверь загрузчик кнопкой «Проверить».").arg(exitCode)
                    : tr("yt-dlp завершился с кодом %1.").arg(exitCode);
            const QString details =
                DownloaderErrors::humanizeDownloaderError(m_stderr, m_settings ? m_settings->browser() : QString());
            if (!details.isEmpty()) {
                message += QStringLiteral(" ") + details;
            }
            finishError(message);
            return;
        }
        parseMetadata(m_stdout);
    });

    QStringList arguments = {QStringLiteral("--ignore-config"),
                             QStringLiteral("--socket-timeout"),
                             QStringLiteral("10"),
                             QStringLiteral("--retries"),
                             QStringLiteral("1"),
                             QStringLiteral("--extractor-retries"),
                             QStringLiteral("1"),
                             QStringLiteral("--playlist-end"),
                             QStringLiteral("10000"),
                             QStringLiteral("--dump-single-json"),
                             QStringLiteral("--skip-download"),
                             QStringLiteral("--no-warnings"),
                             QStringLiteral("--format"),
                             QStringLiteral("bestaudio/best")};
    if (!QStandardPaths::findExecutable(QStringLiteral("node")).isEmpty())
        arguments << QStringLiteral("--js-runtimes") << QStringLiteral("node");
    arguments.append(QStringLiteral("--flat-playlist"));
    if (m_auth) {
        arguments.append(m_auth->sessionArguments(service, authorized));
    }
    arguments << QStringLiteral("--") << source;
    m_process->start(program, arguments, QIODevice::ReadOnly);
}

void ImportPreviewer::inspectSpotify(const QString& url)
{
    beginResetModel();
    Item item;
    item.title = tr("Ссылка Spotify (предпросмотр метаданных недоступен)");
    item.url = url;
    m_items.append(item);
    endResetModel();
    updateSummary();
    setBusy(false);
    setStatusText(tr("Spotify будет загружен через spotdl; длительность и размер до загрузки неизвестны."));
}

void ImportPreviewer::parseMetadata(const QByteArray& json)
{
    QJsonParseError parseError{};
    const QJsonDocument document = QJsonDocument::fromJson(json, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        finishError(tr("Не удалось разобрать ответ yt-dlp: %1").arg(parseError.errorString()));
        return;
    }

    const QJsonObject root = document.object();
    QVector<QJsonObject> objects;
    const QJsonValue entries = root.value(QStringLiteral("entries"));
    if (entries.isArray()) {
        const QJsonArray array = entries.toArray();
        for (const QJsonValue& value : array) {
            if (value.isObject()) {
                objects.append(value.toObject());
            }
        }
    } else {
        objects.append(root);
    }
    if (objects.isEmpty()) {
        finishError(tr("Сервис не вернул треки для предпросмотра."));
        return;
    }

    QVector<Item> parsedItems;
    for (const QJsonObject& object : std::as_const(objects)) {
        Item item;
        item.selected = !m_searching;
        const QString id = object.value(QStringLiteral("id")).toVariant().toString();
        item.title = object.value(QStringLiteral("title")).toString();
        if (item.title.isEmpty()) {
            item.title = id.isEmpty() ? m_sourceUrl : id;
        }
        item.artist = object.value(QStringLiteral("artist")).toString();
        if (item.artist.isEmpty()) {
            item.artist = object.value(QStringLiteral("uploader")).toString();
        }
        item.url = object.value(QStringLiteral("webpage_url")).toString();
        if (item.url.isEmpty())
            item.url = object.value(QStringLiteral("url")).toString();
        if (m_downloads->serviceForUrl(item.url).isEmpty())
            item.url = object.value(QStringLiteral("original_url")).toString();
        if (item.url.isEmpty() && !id.isEmpty() &&
            m_downloads->serviceForUrl(m_sourceUrl) == QStringLiteral("YouTube")) {
            item.url = QStringLiteral("https://www.youtube.com/watch?v=%1").arg(id);
        }
        // Если прямой ссылки нет, ищем тот же трек на YouTube по исполнителю и названию.
        if (m_downloads->serviceForUrl(item.url).isEmpty()) {
            if (!item.artist.isEmpty() && !item.title.isEmpty()) {
                item.url = QStringLiteral("ytsearch1:") + item.artist + QStringLiteral(" - ") + item.title;
            }
        }
        if (m_downloads->serviceForUrl(item.url).isEmpty()) {
            continue;
        }

        const double durationSeconds = object.value(QStringLiteral("duration")).toDouble(0.0);
        item.duration = durationSeconds > 0.0 ? static_cast<qint64>(durationSeconds * 1000.0) : 0;
        if (m_quality == QStringLiteral("original")) {
            item.size = jsonInteger(object, QStringLiteral("filesize"));
            if (item.size < 0) {
                item.size = jsonInteger(object, QStringLiteral("filesize_approx"));
            }
        }
        if (item.size < 0) {
            item.size = estimateBytes(item.duration, m_quality);
        }
        parsedItems.append(item);
    }
    if (parsedItems.isEmpty()) {
        finishError(tr("В ответе сервиса нет ссылок, поддерживаемых для загрузки."));
        return;
    }

    beginResetModel();
    m_items = std::move(parsedItems);
    endResetModel();
    updateSummary();
    setBusy(false);
    setLastError({});
    setStatusText(tr("Найдено треков: %1. Выбрано: %2.").arg(m_items.size()).arg(m_selectedCount));
}

void ImportPreviewer::toggle(int index)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }
    Item& item = m_items[index];
    item.selected = !item.selected;
    emit dataChanged(this->index(index), this->index(index), {SelectedRole});
    updateSummary();
}

void ImportPreviewer::selectAll(bool selected)
{
    if (m_items.isEmpty()) {
        return;
    }
    for (Item& item : m_items) {
        item.selected = selected;
    }
    emit dataChanged(index(0), index(m_items.size() - 1), {SelectedRole});
    updateSummary();
}

void ImportPreviewer::startSelected()
{
    if (m_busy) {
        setLastError(tr("Дождитесь завершения предпросмотра перед загрузкой."));
        return;
    }
    if (!m_downloads) {
        setLastError(tr("Очередь загрузок недоступна."));
        return;
    }
    QStringList urls;
    QStringList titles;
    for (const Item& item : std::as_const(m_items)) {
        if (item.selected) {
            urls.append(item.url);
            titles.append(item.artist.isEmpty() ? item.title : item.artist + QStringLiteral(" - ") + item.title);
        }
    }
    if (urls.isEmpty()) {
        setLastError(tr("Выберите хотя бы один трек для загрузки."));
        return;
    }
    setLastError({});
    m_downloads->enqueue(urls, m_quality, m_authorized, titles);
    setStatusText(tr("Выбранные ссылки переданы в очередь загрузок: %1.").arg(urls.size()));
}

void ImportPreviewer::updateSummary()
{
    qint64 totalDuration = 0;
    qint64 selectedBytes = 0;
    int selectedCount = 0;
    int unknownSizes = 0;
    for (const Item& item : std::as_const(m_items)) {
        if (item.duration > 0 && totalDuration <= std::numeric_limits<qint64>::max() - item.duration) {
            totalDuration += item.duration;
        }
        if (item.size < 0) {
            ++unknownSizes;
        }
        if (item.selected) {
            ++selectedCount;
            if (item.size > 0 && selectedBytes <= std::numeric_limits<qint64>::max() - item.size) {
                selectedBytes += item.size;
            }
        }
    }
    if (m_totalDuration != totalDuration) {
        m_totalDuration = totalDuration;
        emit totalDurationChanged();
    }
    if (m_selectedCount != selectedCount) {
        m_selectedCount = selectedCount;
        emit selectedCountChanged();
    }
    if (m_selectedBytes != selectedBytes) {
        m_selectedBytes = selectedBytes;
        emit selectedBytesChanged();
    }
    if (m_unknownSizes != unknownSizes) {
        m_unknownSizes = unknownSizes;
        emit unknownSizesChanged();
    }
}

void ImportPreviewer::finishError(const QString& message)
{
    if (m_process) {
        disconnect(m_process, nullptr, this, nullptr);
        m_process->deleteLater();
        m_process = nullptr;
    }
    setBusy(false);
    setStatusText(tr("Предпросмотр не выполнен"));
    setLastError(message);
}

void ImportPreviewer::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }
    m_busy = busy;
    emit busyChanged();
}

void ImportPreviewer::setStatusText(const QString& statusText)
{
    if (m_statusText == statusText) {
        return;
    }
    m_statusText = statusText;
    emit statusTextChanged();
}

void ImportPreviewer::setLastError(const QString& lastError)
{
    if (m_lastError == lastError) {
        return;
    }
    m_lastError = lastError;
    emit lastErrorChanged();
}

void ImportPreviewer::setSourceUrl(const QString& sourceUrl)
{
    if (m_sourceUrl == sourceUrl) {
        return;
    }
    m_sourceUrl = sourceUrl;
    emit sourceUrlChanged();
}

void ImportPreviewer::cancel()
{
    if (!m_process)
        return;
    m_stopReason = tr("Получение списка отменено.");
    m_process->kill();
}
