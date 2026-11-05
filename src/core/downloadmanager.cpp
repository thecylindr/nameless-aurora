#include "downloadmanager.h"
#include <QSet>
#include <algorithm>
#include "downloads/nativejob_p.h"
#include "auth/webloginmanager.h"

#include "auth/browserauthmanager.h"
#include "downloads/dependencymanager.h"
#include "downloads/errormapper.h"
#include "importmanager.h"
#include "librarymodel.h"
#include "settingsstore.h"
#include "web/mediasniffer.h"

#include <QDir>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QCryptographicHash>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSaveFile>
#include <QDateTime>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>

#include <utility>

// Реализация очереди загрузок: запуск внешних загрузчиков (yt-dlp/spotdl), нативные
// загрузки VK/Яндекс Музыки, проверка файлов и импорт результатов в библиотеку.
namespace
{

bool matchesDomain(const QString& host, const QString& domain)
{
    return host == domain || host.endsWith(QLatin1Char('.') + domain);
}

bool isFinishedStatus(const QString& status)
{
    return status == QStringLiteral("succeeded") || status == QStringLiteral("failed") ||
           status == QStringLiteral("cancelled") || status == "recovered";
}

QString displayTitle(const QString& url)
{
    const QUrl parsed(url);
    const QString name = QFileInfo(parsed.path()).fileName();
    return name.isEmpty() ? url : name;
}

QString moveWithoutCollision(const QString& source, const QString& libraryFolder)
{
    const QFileInfo sourceInfo(source);
    QDir library(libraryFolder);
    QString target = library.filePath(sourceInfo.fileName());
    int suffix = 1;
    while (QFileInfo::exists(target)) {
        // Если файл с тем же именем уже есть, сверяем содержимое по SHA-256 и удаляем дубликат.
        if (QFileInfo(target).size() == sourceInfo.size()) {
            QFile a(source), b(target);
            if (a.open(QIODevice::ReadOnly) && b.open(QIODevice::ReadOnly)) {
                QCryptographicHash ha(QCryptographicHash::Sha256), hb(QCryptographicHash::Sha256);
                if (ha.addData(&a) && hb.addData(&b) && ha.result() == hb.result()) {
                    a.close();
                    QFile::remove(source);
                    return target;
                }
            }
        }
        const QString base = sourceInfo.completeBaseName();
        const QString extension = sourceInfo.suffix();
        const QString name = extension.isEmpty() ? QStringLiteral("%1 (%2)").arg(base).arg(suffix)
                                                 : QStringLiteral("%1 (%2).%3").arg(base).arg(suffix).arg(extension);
        target = library.filePath(name);
        ++suffix;
    }

    if (QFile::rename(source, target)) {
        return target;
    }
    if (QFile::copy(source, target) && QFile::remove(source)) {
        return target;
    }
    if (QFileInfo::exists(target)) {
        QFile::remove(target);
    }
    return {};
}

} // namespace

DownloadManager::DownloadManager(SettingsStore* settings, ImportManager* importer, DependencyManager* dependencies,
                                 BrowserAuthManager* auth, QObject* parent, QNetworkAccessManager* transport)
    : QAbstractListModel(parent), m_settings(settings), m_importer(importer), m_dependencies(dependencies),
      m_auth(auth), m_library(importer ? importer->library() : nullptr),
      m_network(transport ? transport : new QNetworkAccessManager(this)), m_statusText(tr("Очередь загрузок пуста"))
{
    if (m_dependencies)
        connect(m_dependencies, &DependencyManager::preparationFinished, this, [this](bool success) {
            if (!m_preparingTools)
                return;
            m_preparingTools = false;
            if (success) {
                startQueued();
                return;
            }
            const auto jobs = m_jobs;
            for (auto* job : jobs)
                if (job->status == "queued")
                    finishFailed(job, "Не удалось подготовить загрузку: " + m_dependencies->lastError());
        });
    if (m_auth && m_auth->webLogin())
        connect(m_auth->webLogin(), &WebLoginManager::mediaReady, this,
                [this](int id, const QUrl& url, const QString& error) {
                    const int row = indexOfJob(id);
                    if (row < 0)
                        return;
                    auto* job = m_jobs[row];
                    if (!job->native || job->status != "resolving")
                        return;
                    if (!error.isEmpty()) {
                        finishFailed(job, error);
                        return;
                    }
                    job->webTrack["mediaVerified"] = true;
                    downloadNative(job, url);
                });
}

DownloadManager::~DownloadManager()
{
    for (Job* job : std::as_const(m_jobs)) {
        if (job->process) {
            QObject::disconnect(job->process, nullptr, this, nullptr);
            job->process->kill();
            job->process->waitForFinished(1000);
            delete job->process;
            job->process = nullptr;
        }
        cleanupNative(job);
        removeTempDir(job);
        delete job;
    }
    m_jobs.clear();
}

int DownloadManager::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : m_jobs.size();
}

QVariant DownloadManager::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_jobs.size()) {
        return {};
    }
    const Job* job = m_jobs.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
    case TitleRole:
        return job->title;
    case JobIdRole:
        return job->id;
    case UrlRole:
        return job->url;
    case StatusRole:
        return job->status;
    case ProgressRole:
        return job->progress;
    case ErrorRole:
        return job->error;
    case ServiceRole:
        return job->service;
    case TrackIdRole:
        return job->webTrack.value("trackId", 0);
    default:
        return {};
    }
}

QHash<int, QByteArray> DownloadManager::roleNames() const
{
    return {{JobIdRole, "jobId"},       {TitleRole, "title"},     {UrlRole, "url"},         {StatusRole, "status"},
            {ProgressRole, "progress"}, {ErrorRole, "errorText"}, {ServiceRole, "service"}, {TrackIdRole, "trackId"}};
}

QString DownloadManager::serviceForUrl(const QString& url) const
{
    const QString trimmed = url.trimmed();
    if (trimmed.startsWith(QStringLiteral("scsearch"), Qt::CaseInsensitive))
        return "SoundCloud";
    if (trimmed.startsWith(QStringLiteral("ytsearch"), Qt::CaseInsensitive)) {
        return QStringLiteral("YouTube");
    }
    const QUrl parsed(trimmed);
    if (!parsed.isValid() || parsed.host().isEmpty()) {
        return {};
    }
    const QString scheme = parsed.scheme().toLower();
    if (scheme != QStringLiteral("http") && scheme != QStringLiteral("https")) {
        return {};
    }

    QString host = parsed.host().toLower();
    if (matchesDomain(host, "archive.org"))
        return "Internet Archive";
    if (matchesDomain(host, "bandcamp.com"))
        return "Bandcamp";
    if (host.endsWith(QLatin1Char('.'))) {
        host.chop(1);
    }
    if (matchesDomain(host, QStringLiteral("youtube.com")) || matchesDomain(host, QStringLiteral("youtu.be")) ||
        matchesDomain(host, QStringLiteral("youtube-nocookie.com"))) {
        return QStringLiteral("YouTube");
    }
    if (matchesDomain(host, QStringLiteral("music.yandex.ru")) ||
        matchesDomain(host, QStringLiteral("music.yandex.com")) ||
        matchesDomain(host, QStringLiteral("music.yandex.by")) ||
        matchesDomain(host, QStringLiteral("music.yandex.kz")) ||
        matchesDomain(host, QStringLiteral("music.yandex.uz"))) {
        return QStringLiteral("Яндекс Музыка");
    }
    if (matchesDomain(host, QStringLiteral("vk.com")) || matchesDomain(host, QStringLiteral("vk.ru"))) {
        return QStringLiteral("VK");
    }
    if (matchesDomain(host, QStringLiteral("spotify.com")) || matchesDomain(host, QStringLiteral("spotify.link"))) {
        return QStringLiteral("Spotify");
    }
    if (matchesDomain(host, "soundcloud.com"))
        return "SoundCloud";
    if (matchesDomain(host, "zaycev.net"))
        return "Zaycev.net";
    if (matchesDomain(host, "hitmos.fm"))
        return "Hitmo";
    if (MediaSniffer::isAudioUrl(parsed))
        return "Аудиофайл";
    return {};
}

void DownloadManager::enqueue(const QStringList& urls, const QString& quality, bool authorized,
                              const QStringList& titles)
{
    const QString normalizedQuality = quality.trimmed().toLower();
    if (normalizedQuality != QStringLiteral("original") && normalizedQuality != QStringLiteral("balanced") &&
        normalizedQuality != QStringLiteral("compact")) {
        setLastError(tr("Некорректное качество. Используйте original, balanced или compact."));
        return;
    }
    if (urls.isEmpty()) {
        setLastError(tr("Не указаны ссылки для загрузки."));
        return;
    }

    setLastError({});
    QStringList rejected;
    for (int i = 0; i < urls.size(); ++i) {
        const QString url = urls.at(i).trimmed();
        const QString service = serviceForUrl(url);
        if (service.isEmpty()) {
            rejected.append(url);
            continue;
        }

        const int row = m_jobs.size();
        beginInsertRows(QModelIndex(), row, row);
        Job* job = new Job;
        job->id = m_nextJobId++;
        job->title = titles.value(i).trimmed().isEmpty() ? displayTitle(url) : titles.value(i).trimmed();
        job->url = url;
        job->service = service;
        job->quality = normalizedQuality;
        job->authorized = authorized;
        if (service == "VK" || service == "Яндекс Музыка") {
            // VK и Яндекс Музыку качаем «нативно» (через браузерную сессию), а не через yt-dlp.
            job->native = true;
            job->webTrack = {{"url", url}, {"title", job->title}};
            if (m_library)
                for (const auto& track : m_library->repository().tracks())
                    if (track.sourceUrl == url) {
                        job->webTrack = {{"url", url},
                                         {"title", track.title},
                                         {"artist", track.artist},
                                         {"durationMs", track.duration},
                                         {"trackId", track.id}};
                        break;
                    }
        }
        m_jobs.append(job);
        endInsertRows();
    }

    if (!rejected.isEmpty()) {
        setLastError(
            tr("Некоторые ссылки пропущены: поддерживаются только HTTP(S) YouTube, Яндекс Музыки, VK и Spotify."));
    }
    updateAggregate();
    startQueued();
}

int DownloadManager::indexOfJob(int id) const
{
    for (int i = 0; i < m_jobs.size(); ++i) {
        if (m_jobs.at(i)->id == id) {
            return i;
        }
    }
    return -1;
}

bool DownloadManager::isActive(const Job* job) const
{
    return job && (job->status == QStringLiteral("queued") || job->status == "resolving" ||
                   job->status == QStringLiteral("downloading") || job->status == QStringLiteral("importing"));
}

int DownloadManager::concurrencyLimit() const
{
    // Ограничиваем число одновременных загрузок значением из настроек (от 1 до 4).
    bool ok = false;
    const int configured = m_settings ? m_settings->property("downloadConcurrency").toInt(&ok) : 0;
    return ok ? qBound(1, configured, 4) : 1;
}

void DownloadManager::startQueued()
{
    int running = 0;
    for (const Job* job : std::as_const(m_jobs)) {
        if (job->status == "resolving" || job->status == QStringLiteral("downloading") ||
            job->status == QStringLiteral("importing")) {
            ++running;
        }
    }

    const int limit = concurrencyLimit();
    for (Job* job : std::as_const(m_jobs)) {
        if (running >= limit) {
            break;
        }
        if (job->preparing) {
            job->cancelRequested = true;
            finishCancelled(job);
            return;
        }
        if (job->status == QStringLiteral("queued")) {
            // Если задача завершилась мгновенно (например, ошибкой), не занимаем слот параллелизма.
            const int previousRunning = running;
            startJob(job);
            if (job->status == "resolving" || job->status == QStringLiteral("downloading")) {
                ++running;
            } else if (previousRunning == running && isFinishedStatus(job->status)) {
                continue;
            }
        }
    }
    updateAggregate();
}

void DownloadManager::startJob(Job* job)
{
    if (!job || job->status != QStringLiteral("queued")) {
        return;
    }
    if (m_dependencies && !m_dependencies->runtimeReady()) {
        if (!m_preparingTools) {
            m_preparingTools = true;
            setStatusText("Подготавливаем инструменты загрузки…");
            m_dependencies->ensureReady();
        }
        return;
    }
    if (!m_settings || m_settings->libraryFolder().trimmed().isEmpty()) {
        finishFailed(job, tr("Папка библиотеки не задана в настройках."));
        return;
    }

    const QString libraryFolder = QDir(m_settings->libraryFolder()).absolutePath();
    if (!QDir().mkpath(libraryFolder)) {
        finishFailed(job, tr("Не удалось создать папку библиотеки: %1").arg(libraryFolder));
        return;
    }

    const QString tempRoot = QDir(libraryFolder).filePath(QStringLiteral(".aurora-temp"));
    job->tempDir = QDir(tempRoot).filePath(QString::number(job->id));
    if (QDir(job->tempDir).exists() && !QDir(job->tempDir).removeRecursively()) {
        finishFailed(job, tr("Не удалось очистить временный каталог: %1").arg(job->tempDir));
        return;
    }
    if (!QDir().mkpath(job->tempDir)) {
        finishFailed(job, tr("Не удалось создать временный каталог: %1").arg(job->tempDir));
        return;
    }

    if (job->native) {
        startNative(job);
        return;
    }

    // Spotify качаем через spotdl, остальные сервисы — через yt-dlp.
    const bool spotify = job->service == QStringLiteral("Spotify");
    const QString program =
        m_dependencies ? (spotify ? m_dependencies->spotdlExecutable() : m_dependencies->ytDlpExecutable()) : QString();
    if (program.isEmpty()) {
        finishFailed(job, tr("%1 недоступен. Установите загрузчики через менеджер зависимостей.")
                              .arg(spotify ? QStringLiteral("spotdl") : QStringLiteral("yt-dlp")));
        return;
    }
    const QString ffmpeg = m_dependencies ? m_dependencies->ffmpegExecutable() : QString();
    if (ffmpeg.isEmpty()) {
        finishFailed(job, tr("FFmpeg не найден. Установите FFmpeg и добавьте его в PATH."));
        return;
    }

    const QString jobUrl = job->url;

    QStringList arguments;
    if (spotify) {
        const QString bitrate = job->quality == QStringLiteral("original")   ? QStringLiteral("320k")
                                : job->quality == QStringLiteral("balanced") ? QStringLiteral("192k")
                                                                             : QStringLiteral("128k");
        arguments << QStringLiteral("download") << job->url << QStringLiteral("--output")
                  << QDir(job->tempDir).filePath(QStringLiteral("{artist}/{title}.{output-ext}"))
                  << QStringLiteral("--format") << QStringLiteral("mp3") << QStringLiteral("--bitrate") << bitrate;
    } else {
        arguments << QStringLiteral("--ignore-config") << QStringLiteral("--newline")
                  << QStringLiteral("--no-overwrites") << QStringLiteral("--add-metadata")
                  << QStringLiteral("--embed-thumbnail") << QStringLiteral("--format")
                  << QStringLiteral("bestaudio/best") << QStringLiteral("--ffmpeg-location") << ffmpeg
                  << QStringLiteral("--output")
                  << QDir(job->tempDir).filePath(QStringLiteral("%(title)s [%(id)s].%(ext)s"));
        if (job->webTrack.value("trackId").toInt() > 0)
            arguments << "--no-playlist";
        if (job->quality == QStringLiteral("original"))
            arguments << QStringLiteral("-x");
        if (job->quality != QStringLiteral("original")) {
            arguments << QStringLiteral("-x") << QStringLiteral("--audio-format") << QStringLiteral("mp3")
                      << QStringLiteral("--audio-quality")
                      << (job->quality == QStringLiteral("balanced") ? QStringLiteral("192K") : QStringLiteral("128K"));
        }
        if (m_dependencies)
            arguments.append(m_dependencies->jsRuntimeArguments());
        arguments << "--socket-timeout" << "15" << "--retries" << "2" << "--fragment-retries" << "2"
                  << "--concurrent-fragments" << "2";
        if (m_auth) {
            arguments.append(m_auth->sessionArguments(job->service, job->authorized));
        }
        arguments << QStringLiteral("--") << jobUrl;
    }

    job->cancelRequested = false;
    job->stdoutBuffer.clear();
    job->stderrBuffer.clear();
    job->processError.clear();
    job->process = new QProcess(this);
    job->process->setWorkingDirectory(job->tempDir);
    job->process->setProcessChannelMode(QProcess::SeparateChannels);
    const int id = job->id;

    connect(job->process, &QProcess::readyReadStandardOutput, this, [this, id] {
        const int index = indexOfJob(id);
        if (index >= 0) {
            readProcessOutput(m_jobs.at(index), false);
        }
    });
    connect(job->process, &QProcess::readyReadStandardError, this, [this, id] {
        const int index = indexOfJob(id);
        if (index >= 0) {
            readProcessOutput(m_jobs.at(index), true);
        }
    });
    connect(job->process, &QProcess::errorOccurred, this, [this, id](QProcess::ProcessError error) {
        const int index = indexOfJob(id);
        if (index < 0) {
            return;
        }
        Job* current = m_jobs.at(index);
        if (!current->process) {
            return;
        }
        current->processError = current->process->errorString();
        if (error == QProcess::FailedToStart) {
            if (current->cancelRequested) {
                finishCancelled(current);
            } else {
                finishFailed(current, tr("Не удалось запустить загрузчик: %1").arg(current->processError));
            }
        }
    });
    connect(job->process, &QProcess::finished, this, [this, id](int exitCode, QProcess::ExitStatus exitStatus) {
        const int index = indexOfJob(id);
        if (index < 0) {
            return;
        }
        Job* current = m_jobs.at(index);
        if (!current->process) {
            return;
        }
        readProcessOutput(current, false);
        readProcessOutput(current, true);
        current->process->deleteLater();
        current->process = nullptr;

        if (current->cancelRequested) {
            finishCancelled(current);
            return;
        }
        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
            setJobStatus(current, QStringLiteral("importing"));
            finishSucceeded(current);
            return;
        }
        const auto failure = current->stderrBuffer.toLower();
        // YouTube/SoundCloud могут требовать вход («sign in to confirm», «not a bot») —
        // один раз перезапускаем загрузку с авторизованной сессией браузера.
        if ((current->service == "YouTube" || current->service == "SoundCloud") && !current->sessionRetried && m_auth &&
            (failure.contains("sign in to confirm") || failure.contains("not a bot") ||
             failure.contains("http error 403"))) {
            current->sessionRetried = true;
            current->authorized = true;
            m_auth->grantSessionAccess();
            setJobProgress(current, 0);
            setJobStatus(current, "queued");
            setStatusText("Повторяем загрузку с существующим входом в " + current->service + "…");
            QTimer::singleShot(0, this, &DownloadManager::startQueued);
            return;
        }
        QString details = DownloaderErrors::humanizeDownloaderError(current->stderrBuffer,
                                                                    m_settings ? m_settings->browser() : QString());
        if (details.isEmpty()) {
            details = current->processError;
        }
        QString message = exitStatus == QProcess::CrashExit ? tr("Загрузчик аварийно завершён.")
                                                            : tr("Загрузчик завершился с кодом %1.").arg(exitCode);
        if (!details.isEmpty()) {
            message += QStringLiteral(" ") + details;
        }
        finishFailed(current, message);
    });

    setJobStatus(job, QStringLiteral("downloading"));
    setStatusText(tr("Загрузка из %1…").arg(job->service));
    job->process->start(program, arguments, QIODevice::ReadWrite);
}

void DownloadManager::readProcessOutput(Job* job, bool errorOutput)
{
    if (!job || !job->process) {
        return;
    }
    QString& buffer = errorOutput ? job->stderrBuffer : job->stdoutBuffer;
    buffer.append(QString::fromLocal8Bit(errorOutput ? job->process->readAllStandardError()
                                                     : job->process->readAllStandardOutput()));
    // Держим только хвост вывода и вытаскиваем из него процент прогресса (yt-dlp пишет «NN.N%»).
    buffer = buffer.right(8192);

    static const QRegularExpression percentagePattern(QStringLiteral("(\\d{1,3}(?:[.,]\\d+)?)\\s*%"));
    QRegularExpressionMatchIterator matches = percentagePattern.globalMatch(buffer);
    while (matches.hasNext()) {
        const QRegularExpressionMatch match = matches.next();
        QString value = match.captured(1);
        value.replace(QLatin1Char(','), QLatin1Char('.'));
        bool ok = false;
        const double percentage = value.toDouble(&ok);
        if (ok && percentage >= 0.0 && percentage <= 100.0) {
            setJobProgress(job, percentage / 100.0);
        }
    }
}

void DownloadManager::cancelJob(int id)
{
    const int index = indexOfJob(id);
    if (index < 0) {
        return;
    }
    Job* job = m_jobs.at(index);
    if (job->status == QStringLiteral("queued")) {
        finishCancelled(job);
        return;
    }
    if (job->native && isActive(job)) {
        job->cancelRequested = true;
        setJobStatus(job, "cancelled");
        if (m_auth && m_auth->webLogin())
            m_auth->webLogin()->cancelMedia(id);
        finishCancelled(job);
        return;
    }
    if (job->status != QStringLiteral("downloading") || !job->process) {
        return;
    }
    job->cancelRequested = true;
    setStatusText(tr("Отмена загрузки «%1»…").arg(job->title));
    job->process->kill();
}

void DownloadManager::retryJob(int id)
{
    const int index = indexOfJob(id);
    if (index < 0) {
        return;
    }
    Job* job = m_jobs.at(index);
    if (job->status != QStringLiteral("failed") && job->status != QStringLiteral("cancelled")) {
        setLastError(tr("Повторить можно только неудавшуюся или отменённую загрузку."));
        return;
    }
    const int target = job->webTrack.value("trackId").toInt();
    if (target > 0)
        for (const auto* other : m_jobs)
            if (other != job && other->webTrack.value("trackId").toInt() == target && isActive(other)) {
                setLastError("Этот трек уже загружается из другого источника.");
                return;
            }
    removeTempDir(job);
    job->cancelRequested = false;
    if (job->native)
        job->webTrack.remove("mediaUrl");
    job->processError.clear();
    job->error.clear();
    m_serviceFailures.remove(job->service);
    job->stdoutBuffer.clear();
    job->stderrBuffer.clear();
    setJobProgress(job, 0.0);
    setJobStatus(job, QStringLiteral("queued"));
    setLastError({});
    startQueued();
}

void DownloadManager::clearFinished()
{
    for (int row = m_jobs.size() - 1; row >= 0; --row) {
        Job* job = m_jobs.at(row);
        if (!isFinishedStatus(job->status)) {
            continue;
        }
        beginRemoveRows(QModelIndex(), row, row);
        m_jobs.removeAt(row);
        endRemoveRows();
        removeTempDir(job);
        delete job;
    }
    updateAggregate();
}

void DownloadManager::finishCancelled(Job* job)
{
    cleanupNative(job);
    if (job->process) {
        QObject::disconnect(job->process, nullptr, this, nullptr);
        job->process->deleteLater();
        job->process = nullptr;
    }
    removeTempDir(job);
    setJobProgress(job, 0.0);
    setJobStatus(job, QStringLiteral("cancelled"));
    setStatusText(tr("Загрузка отменена"));
    QTimer::singleShot(0, this, &DownloadManager::startQueued);
    emit trackTransferFinished(job->webTrack.value("trackId").toInt(), false, job->service, "cancelled");
}

void DownloadManager::finishFailed(Job* job, const QString& message)
{
    recordFailure(job, message);
    job->error = message;
    cleanupNative(job);
    if (job->process) {
        QObject::disconnect(job->process, nullptr, this, nullptr);
        job->process->deleteLater();
        job->process = nullptr;
    }
    removeTempDir(job);
    setJobStatus(job, QStringLiteral("failed"));
    setLastError(tr("Загрузка «%1»: %2").arg(job->title, message));
    setStatusText(tr("Загрузка завершилась с ошибкой"));
    if (job->native && (job->service == "VK" || job->service == "Яндекс Музыка") &&
        (message.contains("ожидан", Qt::CaseInsensitive) || message.contains("сесси", Qt::CaseInsensitive) ||
         message.contains("не отдал", Qt::CaseInsensitive))) {
        // После нескольких неудач подряд приостанавливаем очередь сервиса, чтобы не «долбить» его.
        if (++m_serviceFailures[job->service] >= 3) {
            int first = -1, last = -1;
            for (int row = 0; row < m_jobs.size(); ++row) {
                auto* waiting = m_jobs[row];
                if (waiting->native && waiting->service == job->service && waiting->status == "queued") {
                    waiting->status = "failed";
                    waiting->error = "Источник несколько раз не отдал аудио. Очередь этого сервиса приостановлена: "
                                     "выбери другой источник или нажми повтор.";
                    emit trackTransferFinished(waiting->webTrack.value("trackId").toInt(), false, waiting->service,
                                               waiting->error);
                    if (first < 0)
                        first = row;
                    last = row;
                }
            }
            if (first >= 0) {
                emit dataChanged(index(first), index(last), {StatusRole, ErrorRole});
                updateAggregate();
            }
        }
    }
    QTimer::singleShot(0, this, &DownloadManager::startQueued);
    emit trackTransferFinished(job->webTrack.value("trackId").toInt(), false, job->service, message);
}

void DownloadManager::recordFailure(const Job* job, const QString& message)
{
    const QString directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/diagnostics";
    if (!QDir().mkpath(directory))
        return;
    const QString path = directory + "/downloads.json";
    QJsonArray records;
    QFile previous(path);
    if (previous.open(QIODevice::ReadOnly) && previous.size() < 256 * 1024)
        records = QJsonDocument::fromJson(previous.readAll()).array();
    while (records.size() >= 100)
        records.removeFirst();
    const auto text = message.toLower();
    QString category = "source-unavailable";
    if (text.contains("ожидан") || text.contains("timeout"))
        category = "timeout";
    else if (text.contains("403") || text.contains("доступ"))
        category = "access-denied";
    else if (text.contains("подготов") || text.contains("ffmpeg") || text.contains("ffprobe"))
        category = "tools";
    else if (text.contains("фрагмент"))
        category = "fragment";
    else if (text.contains("декод") || text.contains("аудиопоток"))
        category = "invalid-audio";
    else if (text.contains("сесси") || text.contains("вход"))
        category = "session";
    records.append(QJsonObject{{"utc", QDateTime::currentDateTimeUtc().toString(Qt::ISODate)},
                               {"source", job->service},
                               {"stage", job->status},
                               {"trackId", job->webTrack.value("trackId").toInt()},
                               {"native", job->native},
                               {"category", category},
                               {"toolsReady", m_dependencies && m_dependencies->runtimeReady()}});
    // Храним только фиксированные категории и счётчики: вывод загрузчика может содержать
    // подписанные URL медиа, пути к cookies и данные конкретного аккаунта.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return;
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    file.write(QJsonDocument(records).toJson());
    file.commit();
}
void DownloadManager::openDiagnostics()
{
    const auto directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/diagnostics";
    if (QDir().mkpath(directory))
        QDesktopServices::openUrl(QUrl::fromLocalFile(directory));
}

bool DownloadManager::importDownloadedFiles(Job* job, QString* errorMessage)
{
    static const QSet<QString> audioExtensions = {QStringLiteral("mp3"), QStringLiteral("flac"), QStringLiteral("wav"),
                                                  QStringLiteral("ogg"), QStringLiteral("opus"), QStringLiteral("m4a"),
                                                  QStringLiteral("aac"), QStringLiteral("wma"),  QStringLiteral("webm"),
                                                  QStringLiteral("mka"), QStringLiteral("aiff"), QStringLiteral("aif")};
    QStringList sources;
    QDirIterator iterator(job->tempDir, QDir::Files | QDir::Readable | QDir::NoDotAndDotDot,
                          QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        const QString path = iterator.next();
        const QFileInfo info(path);
        if (!info.isSymLink() && audioExtensions.contains(info.suffix().toLower())) {
            sources.append(path);
        }
    }
    sources.sort();
    if (sources.isEmpty()) {
        *errorMessage = tr("Загрузчик завершился успешно, но аудиофайлы во временном каталоге не найдены.");
        return false;
    }

    if (!job->prepared && (job->native || job->webTrack.value("trackId").toInt() > 0) &&
        !validateNativeFile(job, sources.constFirst(), errorMessage))
        return false;
    if (job->webTrack.value("trackId").toInt() > 0 && sources.size() != 1) {
        *errorMessage = tr("Для одного трека источник вернул несколько файлов. Выбери ссылку на одну запись.");
        return false;
    }
    const QString libraryFolder = QDir(m_settings->libraryFolder()).absolutePath();
    QStringList movedFiles;
    for (const QString& source : sources) {
        QString namedSource = source;
        if (job->native || job->webTrack.value("trackId").toInt() > 0) {
            // Именуем файл по названию трека, заменяя недопустимые символы пути.
            QString name = job->title;
            name.replace(QRegularExpression(R"([\\/:*?"<>|\x00-\x1f])"), "_");
            name = name.left(160).trimmed();
            if (name.isEmpty())
                name = "Aurora";
            namedSource = QDir(job->tempDir).filePath(name + "." + QFileInfo(source).suffix());
            if (source != namedSource && !QFile::rename(source, namedSource)) {
                *errorMessage = tr("Не удалось подготовить аудиофайл.");
                return false;
            }
        }
        const QString target = moveWithoutCollision(namedSource, libraryFolder);
        if (target.isEmpty()) {
            *errorMessage = tr("Не удалось переместить файл в библиотеку: %1").arg(QFileInfo(source).fileName());
            return false;
        }
        movedFiles.append(target);
    }

    for (const QString& path : movedFiles) {
        if (job->webTrack.value("trackId").toInt() > 0) {
            const int id = job->webTrack.value("trackId").toInt();
            const bool attached = job->prepared ? m_library->attachPreparedFile(id, path, job->preparedTrack)
                                                : m_library->attachFile(id, path);
            if (!attached) {
                *errorMessage = m_library->lastError();
                return false;
            }
            continue;
        }
        if (m_library->hasFile(path))
            continue;
        if (!m_library->addTrack(path, job->url, job->service)) {
            const QString details = m_library->lastError();
            *errorMessage =
                details.isEmpty()
                    ? tr("Не удалось добавить в библиотеку файл %1.").arg(QFileInfo(path).fileName())
                    : tr("Не удалось добавить в библиотеку файл %1: %2").arg(QFileInfo(path).fileName(), details);
            return false;
        }
    }
    if (movedFiles.size() == 1) {
        job->title = QFileInfo(movedFiles.constFirst()).completeBaseName();
        const int row = indexOfJob(job->id);
        if (row >= 0) {
            emit dataChanged(index(row), index(row), {TitleRole});
        }
    }
    return true;
}

void DownloadManager::finishSucceeded(Job* job)
{
    if (job->preparing)
        return;
    const int trackId = job->webTrack.value("trackId").toInt();
    if (trackId > 0 && !job->prepared) {
        QStringList files;
        QDirIterator it(job->tempDir, QDir::Files | QDir::Readable, QDirIterator::Subdirectories);
        const QSet<QString> formats{"mp3",  "m4a", "opus", "ogg", "webm", "mka",
                                    "flac", "wav", "aac",  "wma", "aiff", "aif"};
        while (it.hasNext()) {
            const auto file = it.next();
            const QFileInfo info(file);
            if (!info.isSymLink() && formats.contains(info.suffix().toLower()))
                files.append(file);
        }
        if (files.size() != 1) {
            finishFailed(job, "Источник должен вернуть один аудиофайл для выбранного трека.");
            return;
        }
        // Валидацию (длительность, метаданные, обложка) делаем в фоновом потоке, чтобы не блокировать UI.
        job->preparing = true;
        const int generation = ++job->preparationGeneration, id = job->id;
        setJobStatus(job, "importing");
        const auto probe =
            m_dependencies ? m_dependencies->ffprobeExecutable() : QStandardPaths::findExecutable("ffprobe");
        const auto decoder =
            m_dependencies ? m_dependencies->ffmpegExecutable() : QStandardPaths::findExecutable("ffmpeg");
        const auto cover = m_library->trackById(trackId).coverPath;
        const double expected = job->webTrack.value("durationMs").toDouble();
        const auto file = files.first();
        using Prepared = QPair<Track, QString>;
        auto* watcher = new QFutureWatcher<Prepared>(this);
        connect(watcher, &QFutureWatcher<Prepared>::finished, this, [this, watcher, id, generation] {
            const auto result = watcher->result();
            watcher->deleteLater();
            const int row = indexOfJob(id);
            if (row < 0)
                return;
            auto* current = m_jobs[row];
            if (!current->preparing || current->preparationGeneration != generation)
                return;
            current->preparing = false;
            if (!result.second.isEmpty()) {
                finishFailed(current, result.second);
                return;
            }
            current->prepared = true;
            current->preparedTrack = result.first;
            finishSucceeded(current);
        });
        watcher->setFuture(QtConcurrent::run([file, expected, probe, decoder, trackId, cover]() -> Prepared {
            QString error;
            if (!validateAudioFile(file, expected, probe, decoder, &error))
                return {{}, error};
            auto metadata = LibraryModel::probeFile(file, probe);
            if (metadata.title.isEmpty())
                return {{}, "Не удалось прочитать метаданные аудио."};
            metadata.coverPath = cover.isEmpty() ? LibraryModel::extractCover(file, trackId, decoder) : cover;
            return {metadata, {}};
        }));
        return;
    }
    QString error;
    if (!importDownloadedFiles(job, &error)) {
        finishFailed(job, error);
        return;
    }
    removeTempDir(job);
    m_serviceFailures.remove(job->service);
    setJobProgress(job, 1.0);
    setJobStatus(job, QStringLiteral("succeeded"));
    const int target = job->webTrack.value("trackId").toInt();
    if (target > 0)
        for (auto* other : m_jobs)
            if (other != job && other->webTrack.value("trackId").toInt() == target && other->status == "failed")
                setJobStatus(other, "recovered");
    setLastError({});
    setStatusText(tr("Загрузка и импорт завершены"));
    QTimer::singleShot(0, this, &DownloadManager::startQueued);
    emit trackTransferFinished(target, true, job->service, {});
}

void DownloadManager::setJobStatus(Job* job, const QString& status)
{
    if (!job || job->status == status) {
        return;
    }
    job->status = status;
    const int row = indexOfJob(job->id);
    if (row >= 0) {
        emit dataChanged(index(row), index(row), {StatusRole, ErrorRole});
    }
    updateAggregate();
}

void DownloadManager::setJobProgress(Job* job, double progress)
{
    const double bounded = qBound(0.0, progress, 1.0);
    if (!job || qFuzzyCompare(job->progress, bounded)) {
        return;
    }
    job->progress = bounded;
    const int row = indexOfJob(job->id);
    if (row >= 0) {
        emit dataChanged(index(row), index(row), {ProgressRole});
    }
    updateAggregate();
}

void DownloadManager::removeTempDir(Job* job)
{
    if (job) {
        ++job->preparationGeneration;
        job->preparing = false;
        job->prepared = false;
        job->preparedTrack = {};
    }
    if (job && !job->tempDir.isEmpty() && QDir(job->tempDir).exists()) {
        QDir(job->tempDir).removeRecursively();
    }
}

void DownloadManager::updateAggregate()
{
    for (const auto* job : std::as_const(m_jobs)) {
        const int id = job->webTrack.value("trackId").toInt();
        if (id > 0 && (job->status == "succeeded" || job->status == "recovered"))
            m_fileBatch[id] = true;
    }
    // Суммарный прогресс — среднее по всем задачам, где завершённые считаются за 100%.
    int active = 0;
    double totalProgress = 0.0;
    for (const Job* job : std::as_const(m_jobs)) {
        if (isActive(job)) {
            ++active;
        }
        totalProgress += job->status == QStringLiteral("succeeded") || job->status == "recovered" ? 1.0 : job->progress;
    }
    const double progress = m_jobs.isEmpty() ? 0.0 : qBound(0.0, totalProgress / m_jobs.size(), 1.0);

    if (m_activeCount != active) {
        m_activeCount = active;
        emit activeCountChanged();
    }
    const bool busy = active > 0;
    if (m_busy != busy) {
        m_busy = busy;
        emit busyChanged();
    }
    if (!qFuzzyCompare(m_progress, progress)) {
        m_progress = progress;
        emit progressChanged();
    }
    emit summaryChanged();
}

void DownloadManager::registerTracks(const QList<int>& ids)
{
    if (!m_library)
        return;
    bool changed = false;
    for (const int id : ids) {
        if (id <= 0 || m_fileBatch.contains(id))
            continue;
        const auto track = m_library->repository().track(id);
        m_fileBatch.insert(id, !track.filePath.isEmpty() && QFileInfo::exists(track.filePath));
        changed = true;
    }
    if (changed)
        emit summaryChanged();
}

int DownloadManager::filesReady() const
{
    return std::count(m_fileBatch.cbegin(), m_fileBatch.cend(), true);
}

int DownloadManager::failedCount() const
{
    QSet<int> seen;
    int count = 0;
    for (auto it = m_jobs.crbegin(); it != m_jobs.crend(); ++it) {
        const auto* job = *it;
        const int trackId = job->webTrack.value("trackId").toInt();
        const int key = trackId > 0 ? trackId : -job->id;
        if (seen.contains(key))
            continue;
        seen.insert(key);
        if (job->status == "failed" && !m_fileBatch.value(trackId))
            ++count;
    }
    return count;
}

int DownloadManager::succeededCount() const
{
    QSet<int> seen;
    for (const auto* job : m_jobs) {
        if (job->status != "succeeded" && job->status != "recovered")
            continue;
        const int trackId = job->webTrack.value("trackId").toInt();
        seen.insert(trackId > 0 ? trackId : -job->id);
    }
    return seen.size();
}
QString DownloadManager::currentTitle() const
{
    for (const auto* job : m_jobs)
        if (job->status == "downloading" || job->status == "resolving" || job->status == "importing")
            return job->title;
    return {};
}
QString DownloadManager::currentService() const
{
    for (const auto* job : m_jobs)
        if (job->status == "downloading" || job->status == "resolving" || job->status == "importing")
            return job->service;
    return {};
}
int DownloadManager::firstFailedTrack() const
{
    for (const auto* job : m_jobs)
        if (job->status == "failed" && job->webTrack.value("trackId").toInt() > 0)
            return job->webTrack.value("trackId").toInt();
    return 0;
}
QVariantMap DownloadManager::trackState(int id) const
{
    for (auto it = m_jobs.crbegin(); it != m_jobs.crend(); ++it) {
        const auto* job = *it;
        if (job->webTrack.value("trackId").toInt() == id)
            return {{"jobId", job->id},
                    {"status", job->status},
                    {"progress", job->progress},
                    {"error", job->error},
                    {"service", job->service}};
    }
    return {};
}
void DownloadManager::cancelAll()
{
    QList<int> ids;
    for (const auto* job : m_jobs)
        if (isActive(job))
            ids.append(job->id);
    for (int id : ids)
        cancelJob(id);
}
void DownloadManager::retryFailed()
{
    QList<int> ids;
    for (const auto* job : m_jobs)
        if (job->status == "failed")
            ids.append(job->id);
    for (int id : ids)
        retryJob(id);
}
void DownloadManager::downloadTrack(int id)
{
    if (!m_library)
        return;
    const auto t = m_library->trackDetails(id);
    if (t.isEmpty() || t.value("downloaded").toBool())
        return;
    if (serviceForUrl(t.value("url").toString()).isEmpty()) {
        setLastError("У трека нет ссылки. Выбери другой источник.");
        return;
    }
    enqueueAlternative(id, {{"url", t.value("url")}, {"source", serviceForUrl(t.value("url").toString())}},
                       m_settings->quality());
}
void DownloadManager::downloadMissing()
{
    if (!m_library || !m_settings)
        return;
    QVariantList native;
    for (const auto& track : m_library->repository().tracks(m_library->searchText(), m_library->playlistFilter())) {
        if (QFileInfo::exists(track.filePath))
            continue;
        const auto service = serviceForUrl(track.sourceUrl);
        if (service == "VK" || service == "Яндекс Музыка")
            native.append(m_library->trackDetails(track.id));
        else if (!service.isEmpty())
            downloadTrack(track.id);
    }
    if (!native.isEmpty())
        enqueueNative(native, m_settings->quality());
}
bool DownloadManager::enqueueAlternative(int id, const QVariantMap& candidate, const QString& quality)
{
    if (!m_library || !m_settings)
        return false;
    const auto selectedQuality = quality.isEmpty() ? m_settings->quality() : quality;
    if (!QStringList{"original", "balanced", "compact"}.contains(selectedQuality))
        return false;
    const auto t = m_library->trackDetails(id);
    const auto url = candidate.value("url").toString();
    const auto service = serviceForUrl(url);
    if (t.isEmpty() || t.value("downloaded").toBool() || service.isEmpty())
        return false;
    for (const auto* job : m_jobs)
        if (job->webTrack.value("trackId").toInt() == id && isActive(job)) {
            setLastError("Этот трек уже загружается.");
            return false;
        }
    auto* job = new Job;
    job->id = m_nextJobId++;
    job->title = t.value("artist").toString() + " — " + t.value("title").toString();
    job->url = url;
    job->service = service;
    job->quality = selectedQuality;
    job->webTrack = t;
    job->webTrack["url"] = url;
    const auto agent = candidate.value("userAgent").toString();
    if (!agent.contains('\r') && !agent.contains('\n'))
        job->webTrack["userAgent"] = agent.left(500);
    const QUrl direct(candidate.value("mediaUrl").toString());
    if (MediaSniffer::isAudioUrl(direct) ||
        (candidate.value("mediaVerified").toBool() && direct.scheme() == "https" && direct.userInfo().isEmpty())) {
        job->webTrack["mediaUrl"] = direct.toString();
        job->webTrack["mediaVerified"] = true;
    } else if (MediaSniffer::isAudioUrl(QUrl(url)))
        job->webTrack["mediaUrl"] = url;
    job->native = service == "VK" || service == "Яндекс Музыка" || job->webTrack.contains("mediaUrl");
    registerTracks({id});
    const int row = m_jobs.size();
    beginInsertRows({}, row, row);
    m_jobs.append(job);
    endInsertRows();
    setLastError({});
    updateAggregate();
    startQueued();
    return true;
}

void DownloadManager::setStatusText(const QString& statusText)
{
    if (m_statusText == statusText) {
        return;
    }
    m_statusText = statusText;
    emit statusTextChanged();
}

void DownloadManager::setLastError(const QString& lastError)
{
    if (m_lastError == lastError) {
        return;
    }
    m_lastError = lastError;
    emit lastErrorChanged();
}
