#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class SettingsStore;

class BrowserAuthManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool sessionGranted READ sessionGranted NOTIFY sessionGrantedChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit BrowserAuthManager(SettingsStore *settings, QObject *parent = nullptr);

    bool sessionGranted() const { return m_sessionGranted; }
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE bool openService(const QString &service);
    Q_INVOKABLE void grantSessionAccess();
    Q_INVOKABLE void revokeSessionAccess();
    Q_INVOKABLE QString consentText(const QString &service) const;

    QStringList sessionArguments(const QString &service, bool authorized) const;

signals:
    void sessionGrantedChanged();
    void lastErrorChanged();

private:
    void setLastError(const QString &lastError);

    SettingsStore *m_settings;
    bool m_sessionGranted = false;
    QString m_lastError;
};
