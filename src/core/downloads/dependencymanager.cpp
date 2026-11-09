#include "dependencymanager.h"

#include "settingsstore.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTimer>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <QMutex>
#include <QMutexLocker>
#include <QDateTime>

// Реализация менеджера зависимостей: определение путей к инструментам, создание venv,
// установка пакетов через pip и перенос FFmpeg/ffprobe в управляемый каталог.
namespace
{

QString executableName(const QString& name)
{
#ifdef Q_OS_WIN
    return name + QStringLiteral(".exe");
#else
    return name;
#endif
}

QString existingExecutable(const QString& path)
{
    const QFileInfo info(path);
    return info.isFile() && info.isExecutable() ? info.absoluteFilePath() : QString();
}

QString findExecutable(const QString& configured, const QStringList& fallbackNames)
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
    for (const QString& name : fallbackNames) {
#ifdef Q_OS_WIN
        const auto bundled = existingExecutable(QDir(QCoreApplication::applicationDirPath()).filePath(executableName(name)));
        if (!bundled.isEmpty())
            return bundled;
#endif
        const QString found = QStandardPaths::findExecutable(name);
        if (!found.isEmpty()) {
            return found;
        }
    }
    return {};
}

QString programVersion(const QString& program, const QStringList& arguments = {QStringLiteral("--version")})
{
    if (program.isEmpty()) {
        return {};
    }
    // Кэшируем версии на 60 секунд; ключ учитывает путь, аргументы, размер и время изменения файла.
    struct Cached
    {
        QString value;
        qint64 utc;
    };
    static QMutex mutex;
    static QHash<QString, Cached> cache;
    const QFileInfo info(program);
    const QString key = program + '\n' + arguments.join('\n') + '\n' + QString::number(info.size()) + '\n' +
                        QString::number(info.lastModified().toMSecsSinceEpoch());
    {
        QMutexLocker lock(&mutex);
        const auto found = cache.constFind(key);
        if (found != cache.cend() && QDateTime::currentMSecsSinceEpoch() - found->utc < 60000)
            return found->value;
    }
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(program, arguments, QIODevice::ReadOnly);
    if (!process.waitForFinished(5000) || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        process.kill();
        process.waitForFinished(1000);
        QMutexLocker lock(&mutex);
        cache[key] = {{}, QDateTime::currentMSecsSinceEpoch()};
        return {};
    }
    const auto value = QString::fromLocal8Bit(process.readAll())
                           .trimmed()
                           .section(QRegularExpression(QStringLiteral("[\\r\\n]")), 0, 0);
    {
        QMutexLocker lock(&mutex);
        cache[key] = {value, QDateTime::currentMSecsSinceEpoch()};
    }
    return value;
}

QString outputTail(QString output)
{
    const QStringList lines = output.split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
    return lines.isEmpty() ? QString() : lines.constLast().trimmed();
}

} // namespace

DependencyManager::DependencyManager(SettingsStore* settings, QObject* parent, bool deferChecks)
    : QObject(parent), m_settings(settings),
      m_toolsDir(
          QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath(QStringLiteral("tools")))
{
    if (!deferChecks) {
        refresh();
        return;
    }
    setBusy(true);
    setStatusText("Проверяем инструменты в фоне…");
    const QStringList programs{ytDlpExecutable(),
                               spotdlExecutable(),
                               ffmpegExecutable(),
                               ffprobeExecutable(),
                               findExecutable({}, {"node"}),
                               findExecutable({}, {"deno"}),
                               existingExecutable(QDir(managedBinDir()).filePath(executableName("node")))};
    auto* watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher] {
        watcher->deleteLater();
        setBusy(false);
        refresh();
        if (m_prepareRequested) {
            m_prepareRequested = false;
            ensureReady();
        }
    });
    watcher->setFuture(QtConcurrent::run([programs] {
        for (int i = 0; i < programs.size(); ++i)
            programVersion(programs[i], {i == 2 || i == 3 ? QStringLiteral("-version") : QStringLiteral("--version")});
    }));
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
    const QString configured = findExecutable(m_settings ? m_settings->ytDlpPath() : QString(), {});
    if (!configured.isEmpty())
        return configured;
    const QString managed =
        existingExecutable(QDir(managedBinDir()).filePath(executableName(QStringLiteral("yt-dlp"))));
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
    const QString configured = findExecutable(m_settings ? m_settings->spotdlPath() : QString(), {});
    if (!configured.isEmpty())
        return configured;
    const QString managed =
        existingExecutable(QDir(managedBinDir()).filePath(executableName(QStringLiteral("spotdl"))));
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
    const QString managed =
        existingExecutable(QDir(managedBinDir()).filePath(executableName(QStringLiteral("ffmpeg"))));
    const auto system = findExecutable({}, {QStringLiteral("ffmpeg")});
    return !managed.isEmpty() ? managed : system;
#endif
}
QString DependencyManager::ffprobeExecutable() const
{
    const auto managed = existingExecutable(QDir(managedBinDir()).filePath(executableName("ffprobe")));
    const auto system = findExecutable({}, {"ffprobe"});
    return !managed.isEmpty() ? managed : system;
}
QStringList DependencyManager::discoverJsRuntime() const
{
    // yt-dlp требует JS-рантайм для некоторых сайтов; используем deno или node нужной версии.
    const auto deno = findExecutable({}, {"deno"});
    if (!deno.isEmpty()) {
        const auto version = programVersion(deno).section(' ', 1, 1).split('.');
        if (!version.isEmpty() &&
            (version[0].toInt() > 2 || (version[0].toInt() == 2 && version.value(1).toInt() >= 3)))
            return {"--js-runtimes", "deno:" + deno};
    }
    const auto managed = existingExecutable(QDir(managedBinDir()).filePath(executableName("node")));
    const auto node = managed.isEmpty() ? findExecutable({}, {"node"}) : managed;
    const auto major = programVersion(node).remove('v').section('.', 0, 0).toInt();
    return node.isEmpty() || major < 22 ? QStringList{} : QStringList{"--js-runtimes", "node:" + node};
}
void DependencyManager::ensureReady()
{
    if (m_busy) {
        m_prepareRequested = true;
        return;
    }
    if (m_runtimeReady) {
        QTimer::singleShot(0, this, [this] {
            emit preparationFinished(true);
        });
        return;
    }
    installOrUpdate();
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
    const QString ytDlpVersion = programVersion(ytDlpExecutable());
    const QString spotdlVersion = programVersion(spotdlExecutable());
    const QString ffmpegVersion = programVersion(ffmpegExecutable(), {QStringLiteral("-version")});
    const QString ffprobeVersion = programVersion(ffprobeExecutable(), {QStringLiteral("-version")});
    const bool ytDlpAvailable = !ytDlpVersion.isEmpty();
    const bool spotdlAvailable = !spotdlVersion.isEmpty();
    const bool ffmpegAvailable = !ffmpegVersion.isEmpty() && !ffprobeVersion.isEmpty();
    m_jsRuntime = discoverJsRuntime();
    const bool runtimeReady = ytDlpAvailable && ffmpegAvailable && !m_jsRuntime.isEmpty();

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
        setStatusText(tr("Инструменты поиска и обработки аудио готовы."));
    } else if (!pythonAvailable) {
        setStatusText(tr("Python не найден. Установите Python 3 для управляемых загрузчиков."));
    } else if (!ffmpegAvailable) {
        setStatusText(tr("FFmpeg или ffprobe не запускается. Установите пакет FFmpeg и проверьте PATH."));
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
        setLastError(
            tr("Python 3 не найден. Установите Python и добавьте его в PATH; глобальные пакеты Aurora не изменяет."));
        setStatusText(tr("Невозможно установить загрузчики без Python."));
        emit preparationFinished(false);
        return;
    }
    if (!QDir().mkpath(m_toolsDir)) {
        setLastError(tr("Не удалось создать каталог инструментов: %1").arg(m_toolsDir));
        emit preparationFinished(false);
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
    auto* stageProcess = m_process;
    QTimer::singleShot(5 * 60 * 1000, stageProcess, [this, stageProcess] {
        if (m_process != stageProcess || stageProcess->state() == QProcess::NotRunning)
            return;
        disconnect(stageProcess, nullptr, this, nullptr);
        stageProcess->kill();
        stageProcess->waitForFinished(1000);
        finishInstall(false, "Подготовка инструментов не завершилась за 5 минут. Проверь соединение и повтори.");
    });

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
        m_outputTail = (m_outputTail + QString::fromLocal8Bit(m_process->readAllStandardOutput()) +
                        QString::fromLocal8Bit(m_process->readAllStandardError()))
                           .right(8192);
        const InstallStage completedStage = m_stage;
        m_process->deleteLater();
        m_process = nullptr;
        m_stage = InstallStage::None;

        if (m_cancelRequested) {
            m_prepareRequested = false;
            setBusy(false);
            setStatusText(tr("Установка отменена."));
            refresh();
            emit preparationFinished(false);
            return;
        }
        if (exitStatus != QProcess::NormalExit || exitCode != 0) {
            QString message = completedStage == InstallStage::CreateVenv
                                  ? tr("Не удалось создать виртуальное окружение Python (код %1).").arg(exitCode)
                                  : tr("Не удалось подготовить загрузчики (код %1).").arg(exitCode);
            const QString details = outputTail(m_outputTail);
            if (!details.isEmpty()) {
                message += QStringLiteral(" ") + details;
            }
            finishInstall(false, message);
            return;
        }
        if (completedStage == InstallStage::CreateVenv) {
            startStage(InstallStage::InstallPackages);
        } else if (completedStage == InstallStage::InstallPackages) {
            startStage(InstallStage::PrepareBinaries);
        } else {
            m_prepareRequested = false;
            setBusy(false);
            setLastError({});
            refresh();
            emit preparationFinished(m_runtimeReady);
        }
    });

    if (stage == InstallStage::CreateVenv) {
        setStatusText(tr("Создание локального виртуального окружения Python…"));
        m_process->start(
            systemPythonExecutable(),
            {QStringLiteral("-m"), QStringLiteral("venv"), QDir(m_toolsDir).filePath(QStringLiteral("venv"))},
            QIODevice::ReadWrite);
    } else if (stage == InstallStage::InstallPackages) {
        const QString python = managedPythonExecutable();
        if (python.isEmpty()) {
            finishInstall(false, tr("Виртуальное окружение создано, но его Python не найден."));
            return;
        }
        setStatusText(tr("Подготавливаем поиск и загрузку музыки…"));
        QStringList packages{"yt-dlp[default]", "secretstorage", "pycryptodome", "keyring"};
        // Если системного FFmpeg нет, ставим переносимую сборку через static-ffmpeg.
        if (!m_ffmpegAvailable)
            packages << "static-ffmpeg>=3,<4";
        // Если JS-рантайма нет в системе, ставим node из wheel-пакета.
        if (jsRuntimeArguments().isEmpty())
            packages << "nodejs-wheel>=22,<26";
        m_process->start(python,
                         QStringList{QStringLiteral("-m"), QStringLiteral("pip"), QStringLiteral("install"),
                                     QStringLiteral("--disable-pip-version-check"), QStringLiteral("--upgrade")} +
                             packages,
                         QIODevice::ReadWrite);
    } else {
        setStatusText(tr("Проверяем обработку аудио…"));
        // Скрипт static_ffmpeg копирует нужные бинарники FFmpeg/ffprobe в управляемый каталог.
        const QString bootstrap =
            "from static_ffmpeg import run; import pathlib, shutil, sys; "
            "paths=run.get_or_fetch_platform_executables_else_raise(); target=pathlib.Path(sys.argv[1]); "
            "[(shutil.copy2(src,target/pathlib.Path(src).name)) for src in paths]";
        m_process->start(managedPythonExecutable(),
                         {"-c", m_ffmpegAvailable ? "print('Audio tools ready')" : bootstrap, managedBinDir()});
    }
#endif
}

void DependencyManager::finishInstall(bool success, const QString& message)
{
    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
    m_stage = InstallStage::None;
    m_prepareRequested = false;
    setBusy(false);
    if (success) {
        setLastError({});
        refresh();
    } else {
        setStatusText(tr("Установка загрузчиков не завершена."));
        setLastError(message);
        emit preparationFinished(false);
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

void DependencyManager::setStatusText(const QString& statusText)
{
    if (m_statusText == statusText) {
        return;
    }
    m_statusText = statusText;
    emit statusTextChanged();
}

void DependencyManager::setLastError(const QString& lastError)
{
    if (m_lastError == lastError) {
        return;
    }
    m_lastError = lastError;
    emit lastErrorChanged();
}
