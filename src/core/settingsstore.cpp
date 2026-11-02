#include "settingsstore.h"

#include <QDir>
#include <QStandardPaths>
#include <QUrl>

SettingsStore::SettingsStore(QObject *parent) : QObject(parent) {
    const QString musicFolder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    m_libraryFolder = m_settings.value(QStringLiteral("libraryFolder"),
                                      QDir(musicFolder).filePath(QStringLiteral("Aurora Player"))).toString();
    m_volume = qBound(0.0, m_settings.value(QStringLiteral("volume"), 0.75).toDouble(), 1.0);
    m_quality = m_settings.value(QStringLiteral("quality"), QStringLiteral("balanced")).toString();
    m_downloadConcurrency = qBound(1, m_settings.value(QStringLiteral("downloadConcurrency"), 1).toInt(), 4);
    m_browser = m_settings.value(QStringLiteral("browser"), QStringLiteral("firefox")).toString();
    m_browserProfile = m_settings.value(QStringLiteral("browserProfile")).toString();
    m_confirmBulkDownload = m_settings.value(QStringLiteral("confirmBulkDownload"), true).toBool();
    m_ytDlpPath = m_settings.value(QStringLiteral("ytDlpPath")).toString();
    m_spotdlPath = m_settings.value(QStringLiteral("spotdlPath")).toString();
    QDir().mkpath(m_libraryFolder);
}

void SettingsStore::setLibraryFolder(const QString &folder) {
    const QString localPath = folder.startsWith(QStringLiteral("file:")) ? QUrl(folder).toLocalFile() : folder;
    const QString cleaned = QDir::cleanPath(localPath);
    if (cleaned.isEmpty() || cleaned == m_libraryFolder)
        return;
    m_libraryFolder = cleaned;
    QDir().mkpath(m_libraryFolder);
    m_settings.setValue(QStringLiteral("libraryFolder"), m_libraryFolder);
    emit libraryFolderChanged();
}

void SettingsStore::setVolume(double volume) {
    const double bounded = qBound(0.0, volume, 1.0);
    if (qFuzzyCompare(m_volume, bounded))
        return;
    m_volume = bounded;
    m_settings.setValue(QStringLiteral("volume"), m_volume);
    emit volumeChanged();
}

void SettingsStore::setQuality(const QString &quality) {
    if (quality != QStringLiteral("original") && quality != QStringLiteral("balanced")
        && quality != QStringLiteral("compact"))
        return;
    if (quality == m_quality)
        return;
    m_quality = quality;
    m_settings.setValue(QStringLiteral("quality"), m_quality);
    emit qualityChanged();
}

void SettingsStore::setDownloadConcurrency(int concurrency) {
    const int bounded = qBound(1, concurrency, 4);
    if (bounded == m_downloadConcurrency)
        return;
    m_downloadConcurrency = bounded;
    m_settings.setValue(QStringLiteral("downloadConcurrency"), bounded);
    emit downloadConcurrencyChanged();
}

void SettingsStore::setBrowser(const QString &browser) {
    const QString normalized = browser.trimmed().toLower();
    if (normalized.isEmpty() || normalized == m_browser)
        return;
    m_browser = normalized;
    m_settings.setValue(QStringLiteral("browser"), normalized);
    emit browserChanged();
}

void SettingsStore::setBrowserProfile(const QString &profile) {
    const QString cleaned = profile.trimmed();
    if (cleaned == m_browserProfile)
        return;
    m_browserProfile = cleaned;
    m_settings.setValue(QStringLiteral("browserProfile"), cleaned);
    emit browserProfileChanged();
}

void SettingsStore::setConfirmBulkDownload(bool confirm) {
    if (confirm == m_confirmBulkDownload)
        return;
    m_confirmBulkDownload = confirm;
    m_settings.setValue(QStringLiteral("confirmBulkDownload"), confirm);
    emit confirmBulkDownloadChanged();
}

void SettingsStore::setYtDlpPath(const QString &path) {
    if (path == m_ytDlpPath)
        return;
    m_ytDlpPath = path;
    m_settings.setValue(QStringLiteral("ytDlpPath"), path);
    emit ytDlpPathChanged();
}

void SettingsStore::setSpotdlPath(const QString &path) {
    if (path == m_spotdlPath)
        return;
    m_spotdlPath = path;
    m_settings.setValue(QStringLiteral("spotdlPath"), path);
    emit spotdlPathChanged();
}

void SettingsStore::resetPreferences() {
    setVolume(0.75);
    setQuality(QStringLiteral("balanced"));
    setDownloadConcurrency(1);
    setBrowser(QStringLiteral("firefox"));
    setBrowserProfile({});
    setConfirmBulkDownload(true);
}
