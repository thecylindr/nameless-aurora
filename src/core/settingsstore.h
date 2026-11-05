#pragma once
// Хранилище настроек приложения поверх QSettings: внешний вид, звук,
// пути загрузчиков и токены сервисов.

#include <QObject>
#include <QSettings>

class SettingsStore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString colorScheme READ colorScheme WRITE setColorScheme NOTIFY appearanceChanged)
    Q_PROPERTY(QString customAccent READ customAccent WRITE setCustomAccent NOTIFY appearanceChanged)
    Q_PROPERTY(QString sphereStyle READ sphereStyle WRITE setSphereStyle NOTIFY appearanceChanged)
    Q_PROPERTY(bool motionEnabled READ motionEnabled WRITE setMotionEnabled NOTIFY appearanceChanged)
    Q_PROPERTY(
        double atmosphereIntensity READ atmosphereIntensity WRITE setAtmosphereIntensity NOTIFY appearanceChanged)
    Q_PROPERTY(bool beatEffects READ beatEffects WRITE setBeatEffects NOTIFY appearanceChanged)
    Q_PROPERTY(double reactionStrength READ reactionStrength WRITE setReactionStrength NOTIFY appearanceChanged)
    Q_PROPERTY(QString atmosphereScene READ atmosphereScene WRITE setAtmosphereScene NOTIFY appearanceChanged)
    Q_PROPERTY(double effectSpeed READ effectSpeed WRITE setEffectSpeed NOTIFY appearanceChanged)
    Q_PROPERTY(double objectDensity READ objectDensity WRITE setObjectDensity NOTIFY appearanceChanged)
    Q_PROPERTY(double sphereDeformation READ sphereDeformation WRITE setSphereDeformation NOTIFY appearanceChanged)
    Q_PROPERTY(double objectScale READ objectScale WRITE setObjectScale NOTIFY appearanceChanged)
    Q_PROPERTY(double decorationOpacity READ decorationOpacity WRITE setDecorationOpacity NOTIFY appearanceChanged)
    Q_PROPERTY(bool compactNavigation READ compactNavigation WRITE setCompactNavigation NOTIFY appearanceChanged)
    Q_PROPERTY(bool sideQueueVisible READ sideQueueVisible WRITE setSideQueueVisible NOTIFY appearanceChanged)
    Q_PROPERTY(bool homeHeroVisible READ homeHeroVisible WRITE setHomeHeroVisible NOTIFY appearanceChanged)
    Q_PROPERTY(bool compactPlayer READ compactPlayer WRITE setCompactPlayer NOTIFY appearanceChanged)
    Q_PROPERTY(QString libraryFolder READ libraryFolder WRITE setLibraryFolder NOTIFY libraryFolderChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(QString quality READ quality WRITE setQuality NOTIFY qualityChanged)
    Q_PROPERTY(
        int downloadConcurrency READ downloadConcurrency WRITE setDownloadConcurrency NOTIFY downloadConcurrencyChanged)
    Q_PROPERTY(QString browser READ browser WRITE setBrowser NOTIFY browserChanged)
    Q_PROPERTY(QString browserProfile READ browserProfile WRITE setBrowserProfile NOTIFY browserProfileChanged)
    Q_PROPERTY(bool confirmBulkDownload READ confirmBulkDownload WRITE setConfirmBulkDownload NOTIFY
                   confirmBulkDownloadChanged)
    Q_PROPERTY(QString ytDlpPath READ ytDlpPath WRITE setYtDlpPath NOTIFY ytDlpPathChanged)
    Q_PROPERTY(QString spotdlPath READ spotdlPath WRITE setSpotdlPath NOTIFY spotdlPathChanged)
    Q_PROPERTY(QString vkClientId READ vkClientId WRITE setVkClientId NOTIFY vkClientIdChanged)
    Q_PROPERTY(QString vkClientSecret READ vkClientSecret WRITE setVkClientSecret NOTIFY vkClientSecretChanged)
    Q_PROPERTY(QString vkToken READ vkToken WRITE setVkToken NOTIFY vkTokenChanged)
    Q_PROPERTY(QString yandexClientId READ yandexClientId WRITE setYandexClientId NOTIFY yandexClientIdChanged)
    Q_PROPERTY(
        QString yandexClientSecret READ yandexClientSecret WRITE setYandexClientSecret NOTIFY yandexClientSecretChanged)
    Q_PROPERTY(QString yandexToken READ yandexToken WRITE setYandexToken NOTIFY yandexTokenChanged)
    Q_PROPERTY(
        QString yandexRefreshToken READ yandexRefreshToken WRITE setYandexRefreshToken NOTIFY yandexRefreshTokenChanged)
    Q_PROPERTY(
        qint64 yandexTokenExpiry READ yandexTokenExpiry WRITE setYandexTokenExpiry NOTIFY yandexTokenExpiryChanged)

  public:
    explicit SettingsStore(QObject* parent = nullptr);

    QString colorScheme() const
    {
        return m_colorScheme;
    }
    QString customAccent() const
    {
        return m_customAccent;
    }
    QString sphereStyle() const
    {
        return m_sphereStyle;
    }
    bool motionEnabled() const
    {
        return m_motionEnabled;
    }
    double atmosphereIntensity() const
    {
        return m_atmosphereIntensity;
    }
    bool beatEffects() const
    {
        return m_beatEffects;
    }
    double reactionStrength() const
    {
        return m_reactionStrength;
    }
    void setBeatEffects(bool value);
    void setReactionStrength(double value);
    void setColorScheme(const QString& value);
    void setCustomAccent(const QString& value);
    void setSphereStyle(const QString& value);
    void setMotionEnabled(bool value);
    void setAtmosphereIntensity(double value);
    QString atmosphereScene() const
    {
        return m_atmosphereScene;
    }
    double effectSpeed() const
    {
        return m_effectSpeed;
    }
    void setEffectSpeed(double value);
    double objectDensity() const
    {
        return m_objectDensity;
    }
    bool compactNavigation() const
    {
        return m_compactNavigation;
    }
    bool sideQueueVisible() const
    {
        return m_sideQueueVisible;
    }
    bool homeHeroVisible() const
    {
        return m_homeHeroVisible;
    }
    bool compactPlayer() const
    {
        return m_compactPlayer;
    }
    void setAtmosphereScene(const QString& value);
    void setObjectDensity(double value);
    double sphereDeformation() const
    {
        return m_sphereDeformation;
    }
    void setSphereDeformation(double value);
    double objectScale() const
    {
        return m_objectScale;
    }
    void setObjectScale(double value);
    double decorationOpacity() const
    {
        return m_decorationOpacity;
    }
    void setDecorationOpacity(double value);
    void setCompactNavigation(bool value);
    void setSideQueueVisible(bool value);
    void setHomeHeroVisible(bool value);
    void setCompactPlayer(bool value);
    QString libraryFolder() const
    {
        return m_libraryFolder;
    }
    double volume() const
    {
        return m_volume;
    }
    QString quality() const
    {
        return m_quality;
    }
    int downloadConcurrency() const
    {
        return m_downloadConcurrency;
    }
    QString browser() const
    {
        return m_browser;
    }
    QString browserProfile() const
    {
        return m_browserProfile;
    }
    bool confirmBulkDownload() const
    {
        return m_confirmBulkDownload;
    }
    QString ytDlpPath() const
    {
        return m_ytDlpPath;
    }
    QString spotdlPath() const
    {
        return m_spotdlPath;
    }
    QString vkClientId() const
    {
        return m_vkClientId;
    }
    QString vkClientSecret() const
    {
        return m_vkClientSecret;
    }
    QString vkToken() const
    {
        return m_vkToken;
    }
    QString yandexClientId() const
    {
        return m_yandexClientId;
    }
    QString yandexClientSecret() const
    {
        return m_yandexClientSecret;
    }
    QString yandexToken() const
    {
        return m_yandexToken;
    }
    QString yandexRefreshToken() const
    {
        return m_yandexRefreshToken;
    }
    qint64 yandexTokenExpiry() const
    {
        return m_yandexTokenExpiry;
    }

    void setLibraryFolder(const QString& folder);
    void setVolume(double volume);
    void setQuality(const QString& quality);
    void setDownloadConcurrency(int concurrency);
    void setBrowser(const QString& browser);
    void setBrowserProfile(const QString& profile);
    void setConfirmBulkDownload(bool confirm);
    void setYtDlpPath(const QString& path);
    void setSpotdlPath(const QString& path);
    void setVkClientId(const QString& id);
    void setVkClientSecret(const QString& secret);
    void setVkToken(const QString& token);
    void setYandexClientId(const QString& id);
    void setYandexClientSecret(const QString& secret);
    void setYandexToken(const QString& token);
    void setYandexRefreshToken(const QString& token);
    void setYandexTokenExpiry(qint64 expiry);
    // Сбрасывает все настройки к значениям по умолчанию.
    Q_INVOKABLE void resetPreferences();

  signals:
    void appearanceChanged();
    void libraryFolderChanged();
    void volumeChanged();
    void qualityChanged();
    void downloadConcurrencyChanged();
    void browserChanged();
    void browserProfileChanged();
    void confirmBulkDownloadChanged();
    void ytDlpPathChanged();
    void spotdlPathChanged();
    void vkClientIdChanged();
    void vkClientSecretChanged();
    void vkTokenChanged();
    void yandexClientIdChanged();
    void yandexClientSecretChanged();
    void yandexTokenChanged();
    void yandexRefreshTokenChanged();
    void yandexTokenExpiryChanged();

  private:
    QString m_colorScheme = "aurora", m_customAccent = "#b9a1ff", m_sphereStyle = "ribbons";
    bool m_motionEnabled = true;
    double m_atmosphereIntensity = 0.85;
    bool m_beatEffects = true;
    double m_reactionStrength = 1.15;
    QString m_atmosphereScene = "cosmos";
    double m_objectDensity = .55, m_effectSpeed = 1.;
    double m_sphereDeformation = 1.0;
    double m_objectScale = 1.0;
    double m_decorationOpacity = 1.0;
    bool m_compactNavigation = false, m_sideQueueVisible = true, m_homeHeroVisible = true, m_compactPlayer = false;
    QSettings m_settings;
    QString m_libraryFolder;
    QString m_quality;
    QString m_browser;
    QString m_browserProfile;
    QString m_ytDlpPath;
    QString m_spotdlPath;
    QString m_vkClientId;
    QString m_vkClientSecret;
    QString m_vkToken;
    QString m_yandexClientId;
    QString m_yandexClientSecret;
    QString m_yandexToken;
    QString m_yandexRefreshToken;
    double m_volume = 0.75;
    qint64 m_yandexTokenExpiry = 0;
    int m_downloadConcurrency = 1;
    bool m_confirmBulkDownload = true;
};
