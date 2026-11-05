#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

// Управление инструментами загрузки: поиск/установка Python, yt-dlp, spotdl, FFmpeg
// и JS-рантайма (node/deno) в локальное виртуальное окружение.
class QProcess;
class SettingsStore;

class DependencyManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool pythonAvailable READ pythonAvailable NOTIFY pythonAvailableChanged)
    Q_PROPERTY(bool ffmpegAvailable READ ffmpegAvailable NOTIFY ffmpegAvailableChanged)
    Q_PROPERTY(bool ytDlpAvailable READ ytDlpAvailable NOTIFY ytDlpAvailableChanged)
    Q_PROPERTY(bool spotdlAvailable READ spotdlAvailable NOTIFY spotdlAvailableChanged)
    Q_PROPERTY(bool runtimeReady READ runtimeReady NOTIFY runtimeReadyChanged)
    Q_PROPERTY(QString toolsDir READ toolsDir NOTIFY toolsDirChanged)
    Q_PROPERTY(QString ytDlpVersion READ ytDlpVersion NOTIFY ytDlpVersionChanged)
    Q_PROPERTY(QString spotdlVersion READ spotdlVersion NOTIFY spotdlVersionChanged)
    Q_PROPERTY(QString ffmpegVersion READ ffmpegVersion NOTIFY ffmpegVersionChanged)

  public:
    explicit DependencyManager(SettingsStore* settings, QObject* parent = nullptr, bool deferChecks = false);
    ~DependencyManager() override;

    bool busy() const
    {
        return m_busy;
    }
    QString statusText() const
    {
        return m_statusText;
    }
    QString lastError() const
    {
        return m_lastError;
    }
    bool pythonAvailable() const
    {
        return m_pythonAvailable;
    }
    bool ffmpegAvailable() const
    {
        return m_ffmpegAvailable;
    }
    bool ytDlpAvailable() const
    {
        return m_ytDlpAvailable;
    }
    bool spotdlAvailable() const
    {
        return m_spotdlAvailable;
    }
    bool runtimeReady() const
    {
        return m_runtimeReady;
    }
    QString toolsDir() const
    {
        return m_toolsDir;
    }
    QString ytDlpVersion() const
    {
        return m_ytDlpVersion;
    }
    QString spotdlVersion() const
    {
        return m_spotdlVersion;
    }
    QString ffmpegVersion() const
    {
        return m_ffmpegVersion;
    }

    // Пути к нужным исполняемым файлам (пустая строка — не найдено).
    QString ytDlpExecutable() const;
    QString spotdlExecutable() const;
    QString ffmpegExecutable() const;
    QString ffprobeExecutable() const;
    QStringList jsRuntimeArguments() const
    {
        return m_jsRuntime;
    }
    // Проверяет готовность и при необходимости запускает установку (сигнал preparationFinished).
    Q_INVOKABLE void ensureReady();

    // Перечитывает доступность всех инструментов и их версии.
    Q_INVOKABLE void refresh();
    // Устанавливает или обновляет загрузчики в локальное виртуальное окружение.
    Q_INVOKABLE void installOrUpdate();
    // Отменяет идущую установку.
    Q_INVOKABLE void cancel();

  signals:
    void busyChanged();
    void statusTextChanged();
    void lastErrorChanged();
    void pythonAvailableChanged();
    void ffmpegAvailableChanged();
    void ytDlpAvailableChanged();
    void spotdlAvailableChanged();
    void runtimeReadyChanged();
    void toolsDirChanged();
    void ytDlpVersionChanged();
    void spotdlVersionChanged();
    void ffmpegVersionChanged();
    void preparationFinished(bool success);

  private:
    enum class InstallStage
    {
        None,
        CreateVenv,
        InstallPackages,
        PrepareBinaries
    };

    QString managedBinDir() const;
    QString managedPythonExecutable() const;
    QString systemPythonExecutable() const;
    QStringList discoverJsRuntime() const;
    void startStage(InstallStage stage);
    void finishInstall(bool success, const QString& message);
    void setBusy(bool busy);
    void setStatusText(const QString& statusText);
    void setLastError(const QString& lastError);

    SettingsStore* m_settings;
    QString m_toolsDir;
    bool m_busy = false;
    QString m_statusText;
    QString m_lastError;
    bool m_pythonAvailable = false;
    bool m_ffmpegAvailable = false;
    bool m_ytDlpAvailable = false;
    bool m_spotdlAvailable = false;
    bool m_runtimeReady = false;
    QString m_ytDlpVersion;
    QString m_spotdlVersion;
    QString m_ffmpegVersion;
    QProcess* m_process = nullptr;
    InstallStage m_stage = InstallStage::None;
    bool m_cancelRequested = false;
    QString m_outputTail;
    QStringList m_jsRuntime;
    bool m_prepareRequested = false;
};
