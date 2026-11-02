#pragma once

#include <QObject>
#include <QSettings>

class SettingsStore : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString libraryFolder READ libraryFolder WRITE setLibraryFolder NOTIFY libraryFolderChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(QString quality READ quality WRITE setQuality NOTIFY qualityChanged)
    Q_PROPERTY(int downloadConcurrency READ downloadConcurrency WRITE setDownloadConcurrency NOTIFY downloadConcurrencyChanged)
    Q_PROPERTY(QString browser READ browser WRITE setBrowser NOTIFY browserChanged)
    Q_PROPERTY(QString browserProfile READ browserProfile WRITE setBrowserProfile NOTIFY browserProfileChanged)
    Q_PROPERTY(bool confirmBulkDownload READ confirmBulkDownload WRITE setConfirmBulkDownload NOTIFY confirmBulkDownloadChanged)
    Q_PROPERTY(QString ytDlpPath READ ytDlpPath WRITE setYtDlpPath NOTIFY ytDlpPathChanged)
    Q_PROPERTY(QString spotdlPath READ spotdlPath WRITE setSpotdlPath NOTIFY spotdlPathChanged)

public:
    explicit SettingsStore(QObject *parent = nullptr);

    QString libraryFolder() const { return m_libraryFolder; }
    double volume() const { return m_volume; }
    QString quality() const { return m_quality; }
    int downloadConcurrency() const { return m_downloadConcurrency; }
    QString browser() const { return m_browser; }
    QString browserProfile() const { return m_browserProfile; }
    bool confirmBulkDownload() const { return m_confirmBulkDownload; }
    QString ytDlpPath() const { return m_ytDlpPath; }
    QString spotdlPath() const { return m_spotdlPath; }

    void setLibraryFolder(const QString &folder);
    void setVolume(double volume);
    void setQuality(const QString &quality);
    void setDownloadConcurrency(int concurrency);
    void setBrowser(const QString &browser);
    void setBrowserProfile(const QString &profile);
    void setConfirmBulkDownload(bool confirm);
    void setYtDlpPath(const QString &path);
    void setSpotdlPath(const QString &path);
    Q_INVOKABLE void resetPreferences();

signals:
    void libraryFolderChanged();
    void volumeChanged();
    void qualityChanged();
    void downloadConcurrencyChanged();
    void browserChanged();
    void browserProfileChanged();
    void confirmBulkDownloadChanged();
    void ytDlpPathChanged();
    void spotdlPathChanged();

private:
    QSettings m_settings;
    QString m_libraryFolder;
    QString m_quality;
    QString m_browser;
    QString m_browserProfile;
    QString m_ytDlpPath;
    QString m_spotdlPath;
    double m_volume = 0.75;
    int m_downloadConcurrency = 1;
    bool m_confirmBulkDownload = true;
};
