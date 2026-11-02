#include "downloadmanager.h"

#include "auth/browserauthmanager.h"
#include "downloads/dependencymanager.h"
#include "importmanager.h"
#include "librarymodel.h"
#include "settingsstore.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QTimer>
#include <QUrl>

#include <utility>

namespace {

bool matchesDomain(const QString &host, const QString &domain)
{
    return host == domain || host.endsWith(QLatin1Char('.') + domain);
}

bool isFinishedStatus(const QString &status)
{
    return status == QStringLiteral("succeeded") || status == QStringLiteral("failed")
        || status == QStringLiteral("cancelled");
}

QString lastOutputLine(const QString &output)
{
    const QStringList lines = output.split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
    return lines.isEmpty() ? QString() : lines.constLast().trimmed();
}

QString displayTitle(const QString &url)
{
    const QUrl parsed(url);
    const QString name = QFileInfo(parsed.path()).fileName();
    return name.isEmpty() ? url : name;
}

QString moveWithoutCollision(const QString &source, const QString &libraryFolder)
{
    const QFileInfo sourceInfo(source);
    QDir library(libraryFolder);
    QString target = library.filePath(sourceInfo.fileName());
    int suffix = 1;
    while (QFileInfo::exists(target)) {
        const QString base = sourceInfo.completeBaseName();
        const QString extension = sourceInfo.suffix();
        const QString name = extension.isEmpty()
            ? QStringLiteral("%1 (%2)").arg(base).arg(suffix)
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

}

struct DownloadManager::Job
{
    int id = 0;
    QString title;
    QString url;
    QString service;
    QString quality;
    bool authorized = false;
    QString status = QStringLiteral("queued");
    double progress = 0.0;
    QString tempDir;
    QProcess *process = nullptr;
    bool cancelRequested = false;
    QString stdoutBuffer;
    QString stderrBuffer;
    QString processError;
};

DownloadManager::DownloadManager(SettingsStore *settings, ImportManager *importer,
                                 DependencyManager *dependencies, BrowserAuthManager *auth,
                                 QObject *parent)
    : QAbstractListModel(parent)
    , m_settings(settings)
    , m_importer(importer)
    , m_dependencies(dependencies)
    , m_auth(auth)
    , m_library(importer ? importer->library() : nullptr)
    , m_statusText(tr("Очередь загрузок пуста"))
{
}

DownloadManager::~DownloadManager()
{
    for (Job *job : std::as_const(m_jobs)) {
        if (job->process) {
            QObject::disconnect(job->process, nullptr, this, nullptr);
            job->process->kill();
            job->process->waitForFinished(1000);
            delete job->process;
            job->process = nullptr;
        }
        removeTempDir(job);
        delete job;
    }
    m_jobs.clear();
}

int DownloadManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_jobs.size();
}

QVariant DownloadManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_jobs.size()) {
        return {};
    }
    const Job *job = m_jobs.at(index.row());
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
    default:
        return {};
    }
}

QHash<int, QByteArray> DownloadManager::roleNames() const
{
    return {
        {JobIdRole, "jobId"},
        {TitleRole, "title"},
        {UrlRole, "url"},
        {StatusRole, "status"},
        {ProgressRole, "progress"}
    };
}

QString DownloadManager::serviceForUrl(const QString &url) const
{
    const QUrl parsed(url.trimmed());
    if (!parsed.isValid() || parsed.host().isEmpty()) {
        return {};
    }
    const QString scheme = parsed.scheme().toLower();
    if (scheme != QStringLiteral("http") && scheme != QStringLiteral("https")) {
        return {};
    }

    QString host = parsed.host().toLower();
    if (host.endsWith(QLatin1Char('.'))) {
        host.chop(1);
    }
    if (matchesDomain(host, QStringLiteral("youtube.com"))
        || matchesDomain(host, QStringLiteral("youtu.be"))
        || matchesDomain(host, QStringLiteral("youtube-nocookie.com"))) {
        return QStringLiteral("YouTube");
    }
    if (matchesDomain(host, QStringLiteral("music.yandex.ru"))
        || matchesDomain(host, QStringLiteral("music.yandex.com"))
        || matchesDomain(host, QStringLiteral("music.yandex.by"))
        || matchesDomain(host, QStringLiteral("music.yandex.kz"))
        || matchesDomain(host, QStringLiteral("music.yandex.uz"))) {
        return QStringLiteral("Яндекс Музыка");
    }
    if (matchesDomain(host, QStringLiteral("vk.com")) || matchesDomain(host, QStringLiteral("vk.ru"))) {
        return QStringLiteral("VK");
    }
    if (matchesDomain(host, QStringLiteral("spotify.com"))
        || matchesDomain(host, QStringLiteral("spotify.link"))) {
        return QStringLiteral("Spotify");
    }
    return {};
}

void DownloadManager::enqueue(const QStringList &urls, const QString &quality, bool authorized)
{
    const QString normalizedQuality = quality.trimmed().toLower();
    if (normalizedQuality != QStringLiteral("original") && normalizedQuality != QStringLiteral("balanced")
        && normalizedQuality != QStringLiteral("compact")) {
        setLastError(tr("Некорректное качество. Используйте original, balanced или compact."));
        return;
    }
    if (urls.isEmpty()) {
        setLastError(tr("Не указаны ссылки для загрузки."));
        return;
    }

    setLastError({});
    QStringList rejected;
    for (const QString &rawUrl : urls) {
        const QString url = rawUrl.trimmed();
        const QString service = serviceForUrl(url);
        if (service.isEmpty()) {
            rejected.append(url);
            continue;
        }

        const int row = m_jobs.size();
        beginInsertRows(QModelIndex(), row, row);
        Job *job = new Job;
        job->id = m_nextJobId++;
        job->title = displayTitle(url);
        job->url = url;
        job->service = service;
        job->quality = normalizedQuality;
        job->authorized = authorized;
        m_jobs.append(job);
        endInsertRows();
    }

    if (!rejected.isEmpty()) {
        setLastError(tr("Некоторые ссылки пропущены: поддерживаются только HTTP(S) YouTube, Яндекс Музыки, VK и Spotify."));
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

bool DownloadManager::isActive(const Job *job) const
{
    return job && (job->status == QStringLiteral("queued") || job->status == QStringLiteral("downloading")
                   || job->status == QStringLiteral("importing"));
}

int DownloadManager::concurrencyLimit() const
{
    bool ok = false;
    const int configured = m_settings ? m_settings->property("downloadConcurrency").toInt(&ok) : 0;
    return ok ? qBound(1, configured, 4) : 1;
}

void DownloadManager::startQueued()
{
    int running = 0;
    for (const Job *job : std::as_const(m_jobs)) {
        if (job->status == QStringLiteral("downloading") || job->status == QStringLiteral("importing")) {
            ++running;
        }
    }

    const int limit = concurrencyLimit();
    for (Job *job : std::as_const(m_jobs)) {
        if (running >= limit) {
            break;
        }
        if (job->status == QStringLiteral("queued")) {
            const int previousRunning = running;
            startJob(job);
            if (job->status == QStringLiteral("downloading")) {
                ++running;
            } else if (previousRunning == running && isFinishedStatus(job->status)) {
                continue;
            }
        }
    }
    updateAggregate();
}

void DownloadManager::startJob(Job *job)
{
    if (!job || job->status != QStringLiteral("queued")) {
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

    const bool spotify = job->service == QStringLiteral("Spotify");
    const QString program = m_dependencies
        ? (spotify ? m_dependencies->spotdlExecutable() : m_dependencies->ytDlpExecutable()) : QString();
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

    QStringList arguments;
    if (spotify) {
        const QString bitrate = job->quality == QStringLiteral("original") ? QStringLiteral("320k")
            : job->quality == QStringLiteral("balanced") ? QStringLiteral("192k") : QStringLiteral("128k");
        arguments << QStringLiteral("download") << job->url
                  << QStringLiteral("--output")
                  << QDir(job->tempDir).filePath(QStringLiteral("{artist}/{title}.{output-ext}"))
                  << QStringLiteral("--format") << QStringLiteral("mp3")
                  << QStringLiteral("--bitrate") << bitrate;
    } else {
        arguments << QStringLiteral("--newline") << QStringLiteral("--no-overwrites")
                  << QStringLiteral("--add-metadata") << QStringLiteral("--format")
                  << QStringLiteral("bestaudio/best") << QStringLiteral("--ffmpeg-location") << ffmpeg
                  << QStringLiteral("--output")
                  << QDir(job->tempDir).filePath(QStringLiteral("%(title)s [%(id)s].%(ext)s"));
        if (job->quality != QStringLiteral("original")) {
            arguments << QStringLiteral("-x") << QStringLiteral("--audio-format") << QStringLiteral("mp3")
                      << QStringLiteral("--audio-quality")
                      << (job->quality == QStringLiteral("balanced") ? QStringLiteral("192K")
                                                                     : QStringLiteral("128K"));
        }
        if (m_auth) {
            arguments.append(m_auth->sessionArguments(job->service, job->authorized));
        }
        arguments << QStringLiteral("--") << job->url;
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
        Job *current = m_jobs.at(index);
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
    connect(job->process, &QProcess::finished, this,
            [this, id](int exitCode, QProcess::ExitStatus exitStatus) {
                const int index = indexOfJob(id);
                if (index < 0) {
                    return;
                }
                Job *current = m_jobs.at(index);
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
                QString details = lastOutputLine(current->stderrBuffer);
                if (details.isEmpty()) {
                    details = current->processError;
                }
                QString message = exitStatus == QProcess::CrashExit
                    ? tr("Загрузчик аварийно завершён.")
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

void DownloadManager::readProcessOutput(Job *job, bool errorOutput)
{
    if (!job || !job->process) {
        return;
    }
    QString &buffer = errorOutput ? job->stderrBuffer : job->stdoutBuffer;
    buffer.append(QString::fromLocal8Bit(errorOutput ? job->process->readAllStandardError()
                                                    : job->process->readAllStandardOutput()));
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
    Job *job = m_jobs.at(index);
    if (job->status == QStringLiteral("queued")) {
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
    Job *job = m_jobs.at(index);
    if (job->status != QStringLiteral("failed") && job->status != QStringLiteral("cancelled")) {
        setLastError(tr("Повторить можно только неудавшуюся или отменённую загрузку."));
        return;
    }
    removeTempDir(job);
    job->cancelRequested = false;
    job->processError.clear();
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
        Job *job = m_jobs.at(row);
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

void DownloadManager::finishCancelled(Job *job)
{
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
}

void DownloadManager::finishFailed(Job *job, const QString &message)
{
    if (job->process) {
        QObject::disconnect(job->process, nullptr, this, nullptr);
        job->process->deleteLater();
        job->process = nullptr;
    }
    removeTempDir(job);
    setJobStatus(job, QStringLiteral("failed"));
    setLastError(tr("Загрузка «%1»: %2").arg(job->title, message));
    setStatusText(tr("Загрузка завершилась с ошибкой"));
    QTimer::singleShot(0, this, &DownloadManager::startQueued);
}

bool DownloadManager::importDownloadedFiles(Job *job, QString *errorMessage)
{
    static const QSet<QString> audioExtensions = {
        QStringLiteral("mp3"), QStringLiteral("flac"), QStringLiteral("wav"), QStringLiteral("ogg"),
        QStringLiteral("opus"), QStringLiteral("m4a"), QStringLiteral("aac"), QStringLiteral("wma"),
        QStringLiteral("webm"), QStringLiteral("mka"), QStringLiteral("aiff"), QStringLiteral("aif")
    };
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

    const QString libraryFolder = QDir(m_settings->libraryFolder()).absolutePath();
    QStringList movedFiles;
    for (const QString &source : sources) {
        const QString target = moveWithoutCollision(source, libraryFolder);
        if (target.isEmpty()) {
            *errorMessage = tr("Не удалось переместить файл в библиотеку: %1").arg(QFileInfo(source).fileName());
            return false;
        }
        movedFiles.append(target);
    }

    for (const QString &path : movedFiles) {
        if (!m_library->addTrack(path, job->url, job->service)) {
            const QString details = m_library->lastError();
            *errorMessage = details.isEmpty()
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

void DownloadManager::finishSucceeded(Job *job)
{
    QString error;
    if (!importDownloadedFiles(job, &error)) {
        finishFailed(job, error);
        return;
    }
    removeTempDir(job);
    setJobProgress(job, 1.0);
    setJobStatus(job, QStringLiteral("succeeded"));
    setLastError({});
    setStatusText(tr("Загрузка и импорт завершены"));
    QTimer::singleShot(0, this, &DownloadManager::startQueued);
}

void DownloadManager::setJobStatus(Job *job, const QString &status)
{
    if (!job || job->status == status) {
        return;
    }
    job->status = status;
    const int row = indexOfJob(job->id);
    if (row >= 0) {
        emit dataChanged(index(row), index(row), {StatusRole});
    }
    updateAggregate();
}

void DownloadManager::setJobProgress(Job *job, double progress)
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

void DownloadManager::removeTempDir(Job *job)
{
    if (job && !job->tempDir.isEmpty() && QDir(job->tempDir).exists()) {
        QDir(job->tempDir).removeRecursively();
    }
}

void DownloadManager::updateAggregate()
{
    int active = 0;
    double totalProgress = 0.0;
    for (const Job *job : std::as_const(m_jobs)) {
        if (isActive(job)) {
            ++active;
        }
        totalProgress += job->status == QStringLiteral("succeeded") ? 1.0 : job->progress;
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
}

void DownloadManager::setStatusText(const QString &statusText)
{
    if (m_statusText == statusText) {
        return;
    }
    m_statusText = statusText;
    emit statusTextChanged();
}

void DownloadManager::setLastError(const QString &lastError)
{
    if (m_lastError == lastError) {
        return;
    }
    m_lastError = lastError;
    emit lastErrorChanged();
}
