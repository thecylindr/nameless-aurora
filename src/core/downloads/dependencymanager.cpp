#include "dependencymanager.h"

#include "settingsstore.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

namespace {

constexpr auto kYtDlpVersion = "2025.10.22";
constexpr auto kSpotdlVersion = "4.4.3";

QString executableName(const QString &name)
{
#ifdef Q_OS_WIN
    return name + QStringLiteral(".exe");
#else
    return name;
#endif
}

QString existingExecutable(const QString &path)
{
    const QFileInfo info(path);
    return info.isFile() && info.isExecutable() ? info.absoluteFilePath() : QString();
}

QString findExecutable(const QString &configured, const QStringList &fallbackNames)
{
    if (!configured.trimmed().isEmpty()) {
        const QString found = QStandardPaths::findExecutable(configured.trimmed());
        if (!found.isEmpty()) {
            return found;
        }
        const QString direct = existingExecutable(configured.trimmed());
        if (!direct.isEmpty()) {
            return direct;
        }
    }
    for (const QString &name : fallbackNames) {
        const QString found = QStandardPaths::findExecutable(name);
        if (!found.isEmpty()) {
            return found;
        }
    }
    return {};
}

QString programVersion(const QString &program, const QStringList &arguments = {QStringLiteral("--version")})
{
    if (program.isEmpty()) {
        return {};
    }
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(program, arguments, QIODevice::ReadOnly);
    if (!process.waitForFinished(5000) || process.exitStatus() != QProcess::NormalExit
        || process.exitCode() != 0) {
        process.kill();
        process.waitForFinished(1000);
        return {};
    }
    return QString::fromLocal8Bit(process.readAll()).trimmed().section(QRegularExpression(QStringLiteral("[\\r\\n]")), 0, 0);
}

QString outputTail(QString output)
{
    const QStringList lines = output.split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
    return lines.isEmpty() ? QString() : lines.constLast().trimmed();
}

}

DependencyManager::DependencyManager(SettingsStore *settings, QObject *parent)
    : QObject(parent)
    , m_settings(settings)
    , m_toolsDir(QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath(QStringLiteral("tools")))
{
    refresh();
}

DependencyManager::~DependencyManager()
{
    if (m_process) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

QString DependencyManager::managedBinDir() const
{
#ifdef Q_OS_WIN
    return QDir(m_toolsDir).filePath(QStringLiteral("venv/Scripts"));
#else
    return QDir(m_toolsDir).filePath(QStringLiteral("venv/bin"));
#endif
}

QString DependencyManager::managedPythonExecutable() const
{
    const QDir binDir(managedBinDir());
#ifdef Q_OS_WIN
    return existingExecutable(binDir.filePath(executableName(QStringLiteral("python"))));
#else
    QString path = existingExecutable(binDir.filePath(QStringLiteral("python3")));
    if (path.isEmpty()) {
        path = existingExecutable(binDir.filePath(QStringLiteral("python")));
    }
    return path;
#endif
}

QString DependencyManager::systemPythonExecutable() const
{
#ifdef Q_OS_WIN
    return findExecutable({}, {QStringLiteral("python"), QStringLiteral("py")});
#else
    return findExecutable({}, {QStringLiteral("python3"), QStringLiteral("python")});
#endif
}

QString DependencyManager::ytDlpExecutable() const
{
#ifdef Q_OS_ANDROID
    return {};
#else
    const QString managed = existingExecutable(QDir(managedBinDir()).filePath(executableName(QStringLiteral("yt-dlp"))));
    if (!managed.isEmpty()) {
        return managed;
    }
    return findExecutable(m_settings ? m_settings->ytDlpPath() : QString(), {QStringLiteral("yt-dlp")});
#endif
}

QString DependencyManager::spotdlExecutable() const
{
#ifdef Q_OS_ANDROID
    return {};
#else
    const QString managed = existingExecutable(QDir(managedBinDir()).filePath(executableName(QStringLiteral("spotdl"))));
    if (!managed.isEmpty()) {
        return managed;
    }
    return findExecutable(m_settings ? m_settings->spotdlPath() : QString(), {QStringLiteral("spotdl")});
#endif
}

QString DependencyManager::ffmpegExecutable() const
{
#ifdef Q_OS_ANDROID
    return {};
#else
    const QString managed = existingExecutable(QDir(managedBinDir()).filePath(executableName(QStringLiteral("ffmpeg"))));
    return managed.isEmpty() ? findExecutable({}, {QStringLiteral("ffmpeg")}) : managed;
#endif
}

void DependencyManager::refresh()
{
    if (m_busy) {
        return;
    }
    setBusy(true);
    setLastError({});
#ifdef Q_OS_ANDROID
    m_pythonAvailable = false;
    m_ffmpegAvailable = false;
    m_ytDlpAvailable = false;
    m_spotdlAvailable = false;
    m_runtimeReady = false;
    m_ytDlpVersion.clear();
    m_spotdlVersion.clear();
    m_ffmpegVersion.clear();
    setStatusText(tr("На Android управляемый Python runtime пока недоступен; загрузчики не запускаются."));
    emit pythonAvailableChanged();
    emit ffmpegAvailableChanged();
    emit ytDlpAvailableChanged();
    emit spotdlAvailableChanged();
    emit runtimeReadyChanged();
    emit ytDlpVersionChanged();
    emit spotdlVersionChanged();
    emit ffmpegVersionChanged();
#else
    const bool pythonAvailable = !managedPythonExecutable().isEmpty() || !systemPythonExecutable().isEmpty();
    const bool ytDlpAvailable = !ytDlpExecutable().isEmpty();
    const bool spotdlAvailable = !spotdlExecutable().isEmpty();
    const bool ffmpegAvailable = !ffmpegExecutable().isEmpty();
    const bool runtimeReady = ytDlpAvailable && spotdlAvailable && ffmpegAvailable;
    const QString ytDlpVersion = programVersion(ytDlpExecutable());
    const QString spotdlVersion = programVersion(spotdlExecutable());
    const QString ffmpegVersion = programVersion(ffmpegExecutable());

    if (m_pythonAvailable != pythonAvailable) {
        m_pythonAvailable = pythonAvailable;
        emit pythonAvailableChanged();
    }
    if (m_ytDlpAvailable != ytDlpAvailable) {
        m_ytDlpAvailable = ytDlpAvailable;
        emit ytDlpAvailableChanged();
    }
    if (m_spotdlAvailable != spotdlAvailable) {
        m_spotdlAvailable = spotdlAvailable;
        emit spotdlAvailableChanged();
    }
    if (m_ffmpegAvailable != ffmpegAvailable) {
        m_ffmpegAvailable = ffmpegAvailable;
        emit ffmpegAvailableChanged();
    }
    if (m_runtimeReady != runtimeReady) {
        m_runtimeReady = runtimeReady;
        emit runtimeReadyChanged();
    }
    if (m_ytDlpVersion != ytDlpVersion) {
        m_ytDlpVersion = ytDlpVersion;
        emit ytDlpVersionChanged();
    }
    if (m_spotdlVersion != spotdlVersion) {
        m_spotdlVersion = spotdlVersion;
        emit spotdlVersionChanged();
    }
    if (m_ffmpegVersion != ffmpegVersion) {
        m_ffmpegVersion = ffmpegVersion;
        emit ffmpegVersionChanged();
    }

    if (runtimeReady) {
        setStatusText(tr("Загрузчики готовы к работе."));
    } else if (!pythonAvailable) {
        setStatusText(tr("Python не найден. Установите Python 3 для управляемых загрузчиков."));
    } else if (!ffmpegAvailable) {
        setStatusText(tr("FFmpeg не найден. Установите FFmpeg и добавьте его в PATH."));
    } else {
        setStatusText(tr("Некоторые загрузчики не установлены. Можно установить их локально."));
    }
#endif
    setBusy(false);
}

void DependencyManager::installOrUpdate()
{
    if (m_busy) {
        return;
    }
#ifdef Q_OS_ANDROID
    setLastError(tr("Автоматическая установка на Android пока не поддерживается: требуется bootstrap Python runtime."));
    setStatusText(tr("Установка недоступна на Android."));
    return;
#else
    const QString python = systemPythonExecutable();
    if (python.isEmpty()) {
        setLastError(tr("Python 3 не найден. Установите Python и добавьте его в PATH; глобальные пакеты Aurora не изменяет."));
        setStatusText(tr("Невозможно установить загрузчики без Python."));
        return;
    }
    if (!QDir().mkpath(m_toolsDir)) {
        setLastError(tr("Не удалось создать каталог инструментов: %1").arg(m_toolsDir));
        return;
    }

    m_cancelRequested = false;
    m_outputTail.clear();
    setLastError({});
    setBusy(true);
    startStage(InstallStage::CreateVenv);
#endif
}

void DependencyManager::startStage(InstallStage stage)
{
#ifndef Q_OS_ANDROID
    m_stage = stage;
    m_outputTail.clear();
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::SeparateChannels);

    connect(m_process, &QProcess::readyReadStandardOutput, this, [this] {
        if (m_process) {
            m_outputTail = (m_outputTail + QString::fromLocal8Bit(m_process->readAllStandardOutput())).right(8192);
        }
    });
    connect(m_process, &QProcess::readyReadStandardError, this, [this] {
        if (m_process) {
            m_outputTail = (m_outputTail + QString::fromLocal8Bit(m_process->readAllStandardError())).right(8192);
        }
    });
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart && m_process) {
            const QString details = m_process->errorString();
            finishInstall(false, tr("Не удалось запустить Python: %1").arg(details));
        }
    });
    connect(m_process, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (!m_process) {
            return;
        }
        m_outputTail = (m_outputTail + QString::fromLocal8Bit(m_process->readAllStandardOutput())
                        + QString::fromLocal8Bit(m_process->readAllStandardError())).right(8192);
        const InstallStage completedStage = m_stage;
        m_process->deleteLater();
        m_process = nullptr;
        m_stage = InstallStage::None;

        if (m_cancelRequested) {
            setBusy(false);
            setStatusText(tr("Установка отменена."));
            refresh();
            return;
        }
        if (exitStatus != QProcess::NormalExit || exitCode != 0) {
            QString message = completedStage == InstallStage::CreateVenv
                ? tr("Не удалось создать виртуальное окружение Python (код %1).").arg(exitCode)
                : tr("Не удалось установить yt-dlp и spotdl (код %1).").arg(exitCode);
            const QString details = outputTail(m_outputTail);
            if (!details.isEmpty()) {
                message += QStringLiteral(" ") + details;
            }
            finishInstall(false, message);
            return;
        }
        if (completedStage == InstallStage::CreateVenv) {
            startStage(InstallStage::InstallPackages);
        } else {
            setBusy(false);
            setLastError({});
            refresh();
        }
    });

    if (stage == InstallStage::CreateVenv) {
        setStatusText(tr("Создание локального виртуального окружения Python…"));
        m_process->start(systemPythonExecutable(),
                         {QStringLiteral("-m"), QStringLiteral("venv"),
                          QDir(m_toolsDir).filePath(QStringLiteral("venv"))}, QIODevice::ReadWrite);
    } else {
        const QString python = managedPythonExecutable();
        if (python.isEmpty()) {
            finishInstall(false, tr("Виртуальное окружение создано, но его Python не найден."));
            return;
        }
        setStatusText(tr("Локальная установка yt-dlp и spotdl…"));
        m_process->start(python,
                         {QStringLiteral("-m"), QStringLiteral("pip"), QStringLiteral("install"),
                          QStringLiteral("--disable-pip-version-check"),
                          QStringLiteral("yt-dlp==") + QString::fromLatin1(kYtDlpVersion),
                          QStringLiteral("spotdl==") + QString::fromLatin1(kSpotdlVersion)}, QIODevice::ReadWrite);
    }
#endif
}

void DependencyManager::finishInstall(bool success, const QString &message)
{
    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
    m_stage = InstallStage::None;
    setBusy(false);
    if (success) {
        setLastError({});
        refresh();
    } else {
        setStatusText(tr("Установка загрузчиков не завершена."));
        setLastError(message);
    }
}

void DependencyManager::cancel()
{
    if (!m_busy || !m_process) {
        return;
    }
    m_cancelRequested = true;
    setStatusText(tr("Отмена установки…"));
    m_process->kill();
}

void DependencyManager::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }
    m_busy = busy;
    emit busyChanged();
}

void DependencyManager::setStatusText(const QString &statusText)
{
    if (m_statusText == statusText) {
        return;
    }
    m_statusText = statusText;
    emit statusTextChanged();
}

void DependencyManager::setLastError(const QString &lastError)
{
    if (m_lastError == lastError) {
        return;
    }
    m_lastError = lastError;
    emit lastErrorChanged();
}
