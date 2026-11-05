#include "mpris.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QWindow>
MprisRoot::MprisRoot(QObject* parent, QWindow* window) : QDBusAbstractAdaptor(parent), m_window(window)
{}
void MprisRoot::Raise()
{
    // Показываем и поднимаем окно по запросу медиа-виджета.
    if (m_window) {
        m_window->showNormal();
        m_window->raise();
        m_window->requestActivate();
    }
}
void MprisRoot::Quit()
{
    QCoreApplication::quit();
}
MprisPlayer::MprisPlayer(QObject* parent, PlaybackController* player) : QDBusAbstractAdaptor(parent), m_player(player)
{
    connect(player, &PlaybackController::currentTrackChanged, this, &MprisPlayer::changed);
    connect(player, &PlaybackController::durationChanged, this, &MprisPlayer::changed);
    connect(player, &PlaybackController::seekableChanged, this, &MprisPlayer::changed);
    connect(player, &PlaybackController::playingChanged, this, [this] {
        // Как только началось воспроизведение, перестаём считать плеер «остановленным».
        if (m_player->playing())
            m_stopped = false;
        changed();
    });
    connect(player, &PlaybackController::volumeChanged, this, &MprisPlayer::changed);
    connect(player, &PlaybackController::modesChanged, this, &MprisPlayer::changed);
    connect(player, &PlaybackController::seeked, this, [this](qint64 p) {
        emit Seeked(p * 1000);
    });
}
void MprisPlayer::setLoop(const QString& value)
{
    int i = QStringList{"None", "Playlist", "Track"}.indexOf(value);
    if (i >= 0)
        m_player->setRepeatMode(i);
}
QVariantMap MprisPlayer::metadata() const
{
    if (!m_player->hasTrack())
        return {};
    // Для потоковых треков идентификатор строится из хэша ключа потока, для локальных — из id.
    return {{"mpris:trackid", QVariant::fromValue(QDBusObjectPath(
                                  "/org/mpris/MediaPlayer2/track/" +
                                  (m_player->streaming() ? "online_" + QString::number(qHash(m_player->streamKey()))
                                                         : QString::number(m_player->currentTrackId()))))},
            {"mpris:length", m_player->duration() * 1000},
            {"xesam:title", m_player->currentTitle()},
            {"xesam:artist", QStringList{m_player->currentArtist()}},
            {"mpris:artUrl", m_player->artworkUrl().toString(QUrl::FullyEncoded)}};
}
void MprisPlayer::Next()
{
    // Сохраняем состояние паузы: смена трека не должна включать воспроизведение.
    bool paused = !m_player->playing();
    m_player->next();
    if (paused)
        m_player->pause();
}
void MprisPlayer::Previous()
{
    bool paused = !m_player->playing();
    m_player->previous();
    if (paused)
        m_player->pause();
}
void MprisPlayer::Stop()
{
    m_player->stop();
    m_stopped = true;
    changed();
}
void MprisPlayer::SetPosition(const QDBusObjectPath& track, qlonglong position)
{
    // Применяем позицию только если она относится к текущему треку и лежит в допустимых пределах.
    if (track.path() == "/org/mpris/MediaPlayer2/track/" +
                            (m_player->streaming() ? "online_" + QString::number(qHash(m_player->streamKey()))
                                                   : QString::number(m_player->currentTrackId())) &&
        position >= 0 && position <= m_player->duration() * 1000)
        m_player->seek(position / 1000);
}
void MprisPlayer::changed()
{
    // Рассылаем сигнал PropertiesChanged с актуальными свойствами Player-интерфейса.
    auto message =
        QDBusMessage::createSignal("/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged");
    QVariantMap properties{{"PlaybackStatus", status()}, {"Metadata", metadata()},
                           {"Volume", volume()},         {"LoopStatus", loop()},
                           {"Shuffle", shuffle()},       {"CanPlay", canPlay()},
                           {"CanPause", canPlay()},      {"CanSeek", m_player->seekable()},
                           {"CanGoNext", canPlay()},     {"CanGoPrevious", canPlay()}};
    message << QString("org.mpris.MediaPlayer2.Player") << properties << QStringList();
    QDBusConnection::sessionBus().send(message);
}
void registerMpris(QObject* parent, PlaybackController* player, QWindow* window)
{
    auto* object = new QObject(parent);
    new MprisRoot(object, window);
    new MprisPlayer(object, player);
    auto bus = QDBusConnection::sessionBus();
    // Пытаемся занять основной сервис, а при занятости (второй экземпляр) — уникальный с PID.
    if (!bus.registerService("org.mpris.MediaPlayer2.aurora"))
        bus.registerService("org.mpris.MediaPlayer2.aurora.instance" +
                            QString::number(QCoreApplication::applicationPid()));
    bus.registerObject("/org/mpris/MediaPlayer2", object, QDBusConnection::ExportAdaptors);
}
