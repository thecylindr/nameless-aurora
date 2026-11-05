#pragma once
#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QVariantMap>
#include "playbackcontroller.h"
class QWindow;
// Интеграция с MPRIS (D-Bus): позволяет системным виджетам и медиа-клавишам управлять плеером.
// MprisRoot — корневой интерфейс org.mpris.MediaPlayer2 (Raise/Quit, свойства приложения).
class MprisRoot : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")
    Q_PROPERTY(bool CanQuit READ yes)
    Q_PROPERTY(bool CanRaise READ yes)
    Q_PROPERTY(bool HasTrackList READ no)
    Q_PROPERTY(QString Identity READ identity)
    Q_PROPERTY(QString DesktopEntry READ desktopEntry)
    Q_PROPERTY(QStringList SupportedUriSchemes READ schemes)
    Q_PROPERTY(QStringList SupportedMimeTypes READ mimeTypes)
  public:
    MprisRoot(QObject* parent, QWindow* window);
    bool yes() const
    {
        return true;
    }
    bool no() const
    {
        return false;
    }
    QString identity() const
    {
        return "Aurora";
    }
    QString desktopEntry() const
    {
        return "org.nameless.Aurora";
    }
    QStringList schemes() const
    {
        return {};
    }
    QStringList mimeTypes() const
    {
        return {};
    }
  public slots:
    void Raise();
    void Quit();

  private:
    QWindow* m_window;
};
// MprisPlayer — интерфейс org.mpris.MediaPlayer2.Player: управление воспроизведением и метаданные трека.
class MprisPlayer : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
    Q_PROPERTY(QString PlaybackStatus READ status)
    Q_PROPERTY(QString LoopStatus READ loop WRITE setLoop)
    Q_PROPERTY(double Rate READ rate WRITE setRate)
    Q_PROPERTY(bool Shuffle READ shuffle WRITE setShuffle)
    Q_PROPERTY(QVariantMap Metadata READ metadata)
    Q_PROPERTY(double Volume READ volume WRITE setVolume)
    Q_PROPERTY(qlonglong Position READ position)
    Q_PROPERTY(double MinimumRate READ rate)
    Q_PROPERTY(double MaximumRate READ rate)
    Q_PROPERTY(bool CanGoNext READ canPlay)
    Q_PROPERTY(bool CanGoPrevious READ canPlay)
    Q_PROPERTY(bool CanPlay READ canPlay)
    Q_PROPERTY(bool CanPause READ canPlay)
    Q_PROPERTY(bool CanSeek READ canSeek)
    Q_PROPERTY(bool CanControl READ yes)
  public:
    MprisPlayer(QObject* parent, PlaybackController* player);
    QString status() const
    {
        return m_player->playing() ? "Playing" : m_player->stopped() ? "Stopped" : "Paused";
    }
    QString loop() const
    {
        return QStringList{"None", "Playlist", "Track"}.at(m_player->repeatMode());
    }
    void setLoop(const QString& value);
    double rate() const
    {
        return 1.0;
    }
    void setRate(double)
    {}
    bool shuffle() const
    {
        return m_player->shuffle();
    }
    void setShuffle(bool v)
    {
        m_player->setShuffle(v);
    }
    double volume() const
    {
        return m_player->volume();
    }
    void setVolume(double v)
    {
        m_player->setVolume(v);
    }
    qlonglong position() const
    {
        return m_player->position() * 1000;
    }
    bool canPlay() const
    {
        return m_player->canPlay();
    }
    bool canSeek() const
    {
        return m_player->seekable();
    }
    bool yes() const
    {
        return true;
    }
    QVariantMap metadata() const;
  public slots:
    void Next();
    void Previous();
    void Pause()
    {
        m_player->pause();
    }
    void PlayPause()
    {
        m_player->toggle();
    }
    void Stop();
    void Play()
    {
        m_player->play();
    }
    void Seek(qlonglong offset)
    {
        m_player->seek(m_player->position() + offset / 1000);
    }
    void SetPosition(const QDBusObjectPath& track, qlonglong position);
    void OpenUri(const QString&)
    {}
  signals:
    void Seeked(qlonglong position);

  private:
    void changed();
    PlaybackController* m_player;
    bool m_stopped = true;
};
// Регистрирует оба MPRIS-интерфейса на сессионной шине D-Bus.
void registerMpris(QObject* parent, PlaybackController* player, QWindow* window);
