#include "settingsstore.h"

#include <QColor>
#include <cmath>
#include <algorithm>
#include <QDir>
#include <QStandardPaths>
#include <QUrl>

// Реализация хранилища настроек: загрузка из QSettings, валидация значений
// и сохранение при каждом изменении.

SettingsStore::SettingsStore(QObject* parent) : QObject(parent)
{
    const QString musicFolder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    // Папка медиатеки по умолчанию — «Aurora Player» внутри стандартной папки музыки.
    m_libraryFolder =
        m_settings.value(QStringLiteral("libraryFolder"), QDir(musicFolder).filePath(QStringLiteral("Aurora Player")))
            .toString();
    m_volume = qBound(0.0, m_settings.value(QStringLiteral("volume"), 0.75).toDouble(), 1.0);
    m_quality = m_settings.value(QStringLiteral("quality"), QStringLiteral("balanced")).toString();
    m_downloadConcurrency = qBound(1, m_settings.value(QStringLiteral("downloadConcurrency"), 1).toInt(), 4);
    m_browser = m_settings.value(QStringLiteral("browser"), QStringLiteral("firefox")).toString();
    m_browserProfile = m_settings.value(QStringLiteral("browserProfile")).toString();
    m_confirmBulkDownload = m_settings.value(QStringLiteral("confirmBulkDownload"), true).toBool();
    m_ytDlpPath = m_settings.value(QStringLiteral("ytDlpPath")).toString();
    m_spotdlPath = m_settings.value(QStringLiteral("spotdlPath")).toString();
    m_vkClientId = m_settings.value(QStringLiteral("vkClientId")).toString();
    m_vkClientSecret = m_settings.value(QStringLiteral("vkClientSecret")).toString();
    m_vkToken = m_settings.value(QStringLiteral("vkToken")).toString();
    m_yandexClientId = m_settings.value(QStringLiteral("yandexClientId")).toString();
    m_yandexClientSecret = m_settings.value(QStringLiteral("yandexClientSecret")).toString();
    m_yandexToken = m_settings.value(QStringLiteral("yandexToken")).toString();
    m_yandexRefreshToken = m_settings.value(QStringLiteral("yandexRefreshToken")).toString();
    m_yandexTokenExpiry = m_settings.value(QStringLiteral("yandexTokenExpiry"), 0).toLongLong();
    setColorScheme(m_settings.value("appearance/scheme", "aurora").toString());
    setCustomAccent(m_settings.value("appearance/accent", "#b9a1ff").toString());
    setSphereStyle(m_settings.value("appearance/sphere", "ribbons").toString());
    setMotionEnabled(m_settings.value("appearance/motion", true).toBool());
    setAtmosphereIntensity(m_settings.value("appearance/intensity", 0.85).toDouble());
    setBeatEffects(m_settings.value("appearance/beatEffects", true).toBool());
    setReactionStrength(m_settings.value("appearance/reactionStrength", 1.15).toDouble());
    setAtmosphereScene(m_settings.value("appearance/scene", "cosmos").toString());
    setEffectSpeed(m_settings.value("appearance/effectSpeed", 1.).toDouble());
    setObjectDensity(m_settings.value("appearance/density", .55).toDouble());
    setSphereDeformation(m_settings.value("appearance/sphereDeformation", 1.0).toDouble());
    setObjectScale(m_settings.value("appearance/objectScale", 1.0).toDouble());
    setDecorationOpacity(m_settings.value("appearance/decorationOpacity", 1.0).toDouble());
    setCompactNavigation(m_settings.value("appearance/compactNavigation", false).toBool());
    setSideQueueVisible(m_settings.value("appearance/sideQueue", true).toBool());
    setHomeHeroVisible(m_settings.value("appearance/homeHero", true).toBool());
    setCompactPlayer(m_settings.value("appearance/compactPlayer", false).toBool());
    QDir().mkpath(m_libraryFolder);
}

void SettingsStore::setLibraryFolder(const QString& folder)
{
    // Путь может прийти как «file:///…» — приводим его к локальному виду.
    const QString localPath = folder.startsWith(QStringLiteral("file:")) ? QUrl(folder).toLocalFile() : folder;
    const QString cleaned = QDir::cleanPath(localPath);
    if (cleaned.isEmpty() || cleaned == m_libraryFolder)
        return;
    m_libraryFolder = cleaned;
    QDir().mkpath(m_libraryFolder);
    m_settings.setValue(QStringLiteral("libraryFolder"), m_libraryFolder);
    emit libraryFolderChanged();
}

void SettingsStore::setVolume(double volume)
{
    const double bounded = qBound(0.0, volume, 1.0);
    if (qFuzzyCompare(m_volume, bounded))
        return;
    m_volume = bounded;
    m_settings.setValue(QStringLiteral("volume"), m_volume);
    emit volumeChanged();
}

void SettingsStore::setQuality(const QString& quality)
{
    if (quality != QStringLiteral("original") && quality != QStringLiteral("balanced") &&
        quality != QStringLiteral("compact"))
        return;
    if (quality == m_quality)
        return;
    m_quality = quality;
    m_settings.setValue(QStringLiteral("quality"), m_quality);
    emit qualityChanged();
}

void SettingsStore::setDownloadConcurrency(int concurrency)
{
    const int bounded = qBound(1, concurrency, 4);
    if (bounded == m_downloadConcurrency)
        return;
    m_downloadConcurrency = bounded;
    m_settings.setValue(QStringLiteral("downloadConcurrency"), bounded);
    emit downloadConcurrencyChanged();
}

void SettingsStore::setBrowser(const QString& browser)
{
    const QString normalized = browser.trimmed().toLower();
    if (normalized.isEmpty() || normalized == m_browser)
        return;
    m_browser = normalized;
    m_settings.setValue(QStringLiteral("browser"), normalized);
    emit browserChanged();
}

void SettingsStore::setBrowserProfile(const QString& profile)
{
    const QString cleaned = profile.trimmed();
    if (cleaned == m_browserProfile)
        return;
    m_browserProfile = cleaned;
    m_settings.setValue(QStringLiteral("browserProfile"), cleaned);
    emit browserProfileChanged();
}

void SettingsStore::setConfirmBulkDownload(bool confirm)
{
    if (confirm == m_confirmBulkDownload)
        return;
    m_confirmBulkDownload = confirm;
    m_settings.setValue(QStringLiteral("confirmBulkDownload"), confirm);
    emit confirmBulkDownloadChanged();
}

void SettingsStore::setYtDlpPath(const QString& path)
{
    if (path == m_ytDlpPath)
        return;
    m_ytDlpPath = path;
    m_settings.setValue(QStringLiteral("ytDlpPath"), path);
    emit ytDlpPathChanged();
}

void SettingsStore::setSpotdlPath(const QString& path)
{
    if (path == m_spotdlPath)
        return;
    m_spotdlPath = path;
    m_settings.setValue(QStringLiteral("spotdlPath"), path);
    emit spotdlPathChanged();
}

void SettingsStore::setVkClientId(const QString& id)
{
    const QString cleaned = id.trimmed();
    if (cleaned == m_vkClientId)
        return;
    m_vkClientId = cleaned;
    m_settings.setValue(QStringLiteral("vkClientId"), cleaned);
    emit vkClientIdChanged();
}

void SettingsStore::setVkClientSecret(const QString& secret)
{
    const QString cleaned = secret.trimmed();
    if (cleaned == m_vkClientSecret)
        return;
    m_vkClientSecret = cleaned;
    m_settings.setValue(QStringLiteral("vkClientSecret"), cleaned);
    emit vkClientSecretChanged();
}

void SettingsStore::setVkToken(const QString& token)
{
    const QString cleaned = token.trimmed();
    if (cleaned == m_vkToken)
        return;
    m_vkToken = cleaned;
    m_settings.setValue(QStringLiteral("vkToken"), cleaned);
    emit vkTokenChanged();
}

void SettingsStore::setYandexClientId(const QString& id)
{
    const QString cleaned = id.trimmed();
    if (cleaned == m_yandexClientId)
        return;
    m_yandexClientId = cleaned;
    m_settings.setValue(QStringLiteral("yandexClientId"), cleaned);
    emit yandexClientIdChanged();
}

void SettingsStore::setYandexClientSecret(const QString& secret)
{
    const QString cleaned = secret.trimmed();
    if (cleaned == m_yandexClientSecret)
        return;
    m_yandexClientSecret = cleaned;
    m_settings.setValue(QStringLiteral("yandexClientSecret"), cleaned);
    emit yandexClientSecretChanged();
}

void SettingsStore::setYandexToken(const QString& token)
{
    const QString cleaned = token.trimmed();
    if (cleaned == m_yandexToken)
        return;
    m_yandexToken = cleaned;
    m_settings.setValue(QStringLiteral("yandexToken"), cleaned);
    emit yandexTokenChanged();
}

void SettingsStore::setYandexRefreshToken(const QString& token)
{
    const QString cleaned = token.trimmed();
    if (cleaned == m_yandexRefreshToken)
        return;
    m_yandexRefreshToken = cleaned;
    m_settings.setValue(QStringLiteral("yandexRefreshToken"), cleaned);
    emit yandexRefreshTokenChanged();
}

void SettingsStore::setYandexTokenExpiry(qint64 expiry)
{
    if (expiry == m_yandexTokenExpiry)
        return;
    m_yandexTokenExpiry = expiry;
    m_settings.setValue(QStringLiteral("yandexTokenExpiry"), expiry);
    emit yandexTokenExpiryChanged();
}

// Возвращает настройки к значениям по умолчанию через обычные сеттеры.
void SettingsStore::resetPreferences()
{
    setColorScheme("aurora");
    setCustomAccent("#b9a1ff");
    setSphereStyle("ribbons");
    setMotionEnabled(true);
    setAtmosphereIntensity(0.85);
    setBeatEffects(true);
    setReactionStrength(1.15);
    setAtmosphereScene("cosmos");
    setEffectSpeed(1.);
    setObjectDensity(.55);
    setCompactNavigation(false);
    setSideQueueVisible(true);
    setHomeHeroVisible(true);
    setCompactPlayer(false);
    setVolume(0.75);
    setQuality(QStringLiteral("balanced"));
    setDownloadConcurrency(1);
    setBrowser(QStringLiteral("firefox"));
    setBrowserProfile({});
    setConfirmBulkDownload(true);
    setVkClientId({});
    setVkClientSecret({});
    setVkToken({});
    setYandexClientId({});
    setYandexClientSecret({});
    setYandexToken({});
    setYandexRefreshToken({});
    setYandexTokenExpiry(0);
}

void SettingsStore::setColorScheme(const QString& value)
{
    if (!QStringList{"aurora", "lagoon", "ember", "rose", "midnight", "nameless", "custom"}.contains(value) ||
        value == m_colorScheme)
        return;
    m_colorScheme = value;
    m_settings.setValue("appearance/scheme", value);
    emit appearanceChanged();
}
void SettingsStore::setBeatEffects(bool value)
{
    if (value == m_beatEffects)
        return;
    m_beatEffects = value;
    m_settings.setValue("appearance/beatEffects", value);
    emit appearanceChanged();
}
void SettingsStore::setReactionStrength(double value)
{
    if (!std::isfinite(value))
        return;
    value = std::clamp(value, 0., 2.);
    if (qFuzzyCompare(value, m_reactionStrength))
        return;
    m_reactionStrength = value;
    m_settings.setValue("appearance/reactionStrength", value);
    emit appearanceChanged();
}
void SettingsStore::setCustomAccent(const QString& value)
{
    const QColor color(value);
    if (!color.isValid())
        return;
    const QString normalized = color.name();
    if (normalized == m_customAccent)
        return;
    m_customAccent = normalized;
    m_settings.setValue("appearance/accent", normalized);
    emit appearanceChanged();
}
void SettingsStore::setSphereStyle(const QString& value)
{
    if (!QStringList{"ribbons", "orbit", "liquid"}.contains(value) || value == m_sphereStyle)
        return;
    m_sphereStyle = value;
    m_settings.setValue("appearance/sphere", value);
    emit appearanceChanged();
}
void SettingsStore::setMotionEnabled(bool value)
{
    if (value == m_motionEnabled)
        return;
    m_motionEnabled = value;
    m_settings.setValue("appearance/motion", value);
    emit appearanceChanged();
}
void SettingsStore::setAtmosphereIntensity(double value)
{
    if (!std::isfinite(value))
        return;
    value = qBound(0.0, value, 1.5);
    if (qFuzzyCompare(value, m_atmosphereIntensity))
        return;
    m_atmosphereIntensity = value;
    m_settings.setValue("appearance/intensity", value);
    emit appearanceChanged();
}

void SettingsStore::setAtmosphereScene(const QString& value)
{
    if (!QStringList{"orb", "cosmos", "prisms", "bubbles", "meteors", "ribbons", "bonsai", "off"}.contains(value) ||
        value == m_atmosphereScene)
        return;
    m_atmosphereScene = value;
    m_settings.setValue("appearance/scene", value);
    emit appearanceChanged();
}
void SettingsStore::setObjectDensity(double value)
{
    if (!std::isfinite(value))
        return;
    value = std::clamp(value, 0., 1.);
    if (qFuzzyCompare(value, m_objectDensity))
        return;
    m_objectDensity = value;
    m_settings.setValue("appearance/density", value);
    emit appearanceChanged();
}
void SettingsStore::setCompactNavigation(bool value)
{
    if (value == m_compactNavigation)
        return;
    m_compactNavigation = value;
    m_settings.setValue("appearance/compactNavigation", value);
    emit appearanceChanged();
}
void SettingsStore::setSideQueueVisible(bool value)
{
    if (value == m_sideQueueVisible)
        return;
    m_sideQueueVisible = value;
    m_settings.setValue("appearance/sideQueue", value);
    emit appearanceChanged();
}
void SettingsStore::setHomeHeroVisible(bool value)
{
    if (value == m_homeHeroVisible)
        return;
    m_homeHeroVisible = value;
    m_settings.setValue("appearance/homeHero", value);
    emit appearanceChanged();
}
void SettingsStore::setCompactPlayer(bool value)
{
    if (value == m_compactPlayer)
        return;
    m_compactPlayer = value;
    m_settings.setValue("appearance/compactPlayer", value);
    emit appearanceChanged();
}

void SettingsStore::setEffectSpeed(double value)
{
    if (!std::isfinite(value))
        return;
    value = std::clamp(value, .25, 2.);
    if (qFuzzyCompare(value, m_effectSpeed))
        return;
    m_effectSpeed = value;
    m_settings.setValue("appearance/effectSpeed", value);
    emit appearanceChanged();
}

void SettingsStore::setSphereDeformation(double value)
{
    if (!qIsFinite(value))
        return;
    value = qBound(0.0, value, 3.0);
    if (qFuzzyCompare(value + 1.0, m_sphereDeformation + 1.0))
        return;
    m_sphereDeformation = value;
    m_settings.setValue("appearance/sphereDeformation", value);
    emit appearanceChanged();
}

void SettingsStore::setObjectScale(double value)
{
    if (!qIsFinite(value))
        return;
    value = qBound(0.5, value, 1.8);
    if (qFuzzyCompare(value + 1.0, m_objectScale + 1.0))
        return;
    m_objectScale = value;
    m_settings.setValue("appearance/objectScale", value);
    emit appearanceChanged();
}

void SettingsStore::setDecorationOpacity(double value)
{
    if (!qIsFinite(value))
        return;
    value = qBound(0.0, value, 1.0);
    if (qFuzzyCompare(value + 1.0, m_decorationOpacity + 1.0))
        return;
    m_decorationOpacity = value;
    m_settings.setValue("appearance/decorationOpacity", value);
    emit appearanceChanged();
}
