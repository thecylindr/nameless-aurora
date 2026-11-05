#pragma once
#include <QObject>
#include <QColor>
#include <QFileSystemWatcher>
#include <QTimer>
// Мост к системной теме Nameless (hypr-themes): читает текущую тему и следит за её сменой.
class NamelessBridge : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor accent READ accent NOTIFY themeChanged)
    Q_PROPERTY(QColor secondary READ secondary NOTIFY themeChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY themeChanged)
    Q_PROPERTY(QString themeName READ themeName NOTIFY themeChanged)
  public:
    explicit NamelessBridge(QObject* parent = nullptr);
    QColor accent() const
    {
        return m_accent;
    }
    QColor secondary() const
    {
        return m_secondary;
    }
    bool connected() const
    {
        return m_connected;
    }
    QString themeName() const
    {
        return m_name;
    }
  signals:
    void themeChanged();

  private:
    void reload();
    QColor m_accent{"#b9a1ff"}, m_secondary{"#74d9ce"};
    bool m_connected = false;
    QString m_name;
    QFileSystemWatcher m_watcher;
    QTimer m_debounce;
};
