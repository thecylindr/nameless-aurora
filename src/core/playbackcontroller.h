#pragma once

#include <QObject>
#include <QAudioOutput>
#include <QMediaPlayer>

class LibraryModel;
class SettingsStore;

class PlaybackController : public QObject {
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentTrackChanged)
    Q_PROPERTY(int currentTrackId READ currentTrackId NOTIFY currentTrackChanged)
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY currentTrackChanged)
    Q_PROPERTY(QString currentArtist READ currentArtist NOTIFY currentTrackChanged)
    Q_PROPERTY(QString currentFormat READ currentFormat NOTIFY currentTrackChanged)
    Q_PROPERTY(QString currentCoverPath READ currentCoverPath NOTIFY currentTrackChanged)
    Q_PROPERTY(bool hasTrack READ hasTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    PlaybackController(LibraryModel *library, SettingsStore *settings, QObject *parent = nullptr);
    ~PlaybackController() override;

    int currentIndex() const { return m_currentIndex; }
    int currentTrackId() const { return m_currentTrackId; }
    QString currentTitle() const { return m_currentTitle; }
    QString currentArtist() const { return m_currentArtist; }
    QString currentFormat() const { return m_currentFormat; }
    QString currentCoverPath() const { return m_currentCoverPath; }
    bool hasTrack() const { return m_currentIndex >= 0; }
    bool playing() const { return m_player.playbackState() == QMediaPlayer::PlayingState; }
    qint64 position() const { return m_player.position(); }
    qint64 duration() const { return m_player.duration(); }
    double volume() const { return m_audioOutput.volume(); }
    QString lastError() const { return m_lastError; }

    Q_INVOKABLE void playIndex(int index);
    Q_INVOKABLE void playTrackId(int id);
    Q_INVOKABLE void toggle();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void seek(qint64 position);
    void setVolume(double volume);

signals:
    void currentTrackChanged();
    void playingChanged();
    void positionChanged();
    void durationChanged();
    void volumeChanged();
    void lastErrorChanged();

private:
    void setError(const QString &error);
    void commitListen(bool completed);

    LibraryModel *m_library;
    SettingsStore *m_settings;
    QMediaPlayer m_player;
    QAudioOutput m_audioOutput;
    int m_currentIndex = -1;
    int m_currentTrackId = -1;
    QString m_currentTitle;
    QString m_currentArtist;
    QString m_currentFormat;
    QString m_currentCoverPath;
    QString m_lastError;
};
