#pragma once
// Центральный контроллер воспроизведения: управляет плеером, очередью,
// эквалайзером, аудиореактивностью и атмосферой интерфейса.
#include "audioreactivity.h"
#include "audioequalizer.h"
#include <QAudioSink>
#include <QTimer>
#include <QObject>
#include <memory>
#include <QAudioOutput>
#include <QAudioBufferOutput>
#include <QMediaPlayer>
#include <QElapsedTimer>
#include <QColor>
#include <QVariantList>
#include <QVector>
#include <QImage>
#include <QNetworkAccessManager>
#include "web/streamproxy.h"

class LibraryModel;
class SettingsStore;
class PlaybackController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(AudioEqualizer* equalizer READ equalizer CONSTANT)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentTrackChanged)
    Q_PROPERTY(int currentTrackId READ currentTrackId NOTIFY currentTrackChanged)
    Q_PROPERTY(QString currentTitle READ currentTitle NOTIFY currentTrackChanged)
    Q_PROPERTY(QString currentArtist READ currentArtist NOTIFY currentTrackChanged)
    Q_PROPERTY(QString currentFormat READ currentFormat NOTIFY currentTrackChanged)
    Q_PROPERTY(QString currentCoverPath READ currentCoverPath NOTIFY currentTrackChanged)
    Q_PROPERTY(QUrl artworkUrl READ artworkUrl NOTIFY currentTrackChanged)
    Q_PROPERTY(QColor atmosphereColor READ atmosphereColor NOTIFY currentTrackChanged)
    Q_PROPERTY(QColor atmosphereSecondary READ atmosphereSecondary NOTIFY currentTrackChanged)
    Q_PROPERTY(double atmosphereShape READ atmosphereShape NOTIFY currentTrackChanged)
    Q_PROPERTY(bool favorite READ favorite NOTIFY currentTrackChanged)
    Q_PROPERTY(bool hasTrack READ hasTrack NOTIFY currentTrackChanged)
    Q_PROPERTY(bool streaming READ streaming NOTIFY currentTrackChanged)
    Q_PROPERTY(bool streamLoading READ streamLoading NOTIFY streamLoadingChanged)
    Q_PROPERTY(double streamBufferProgress READ streamBufferProgress NOTIFY streamLoadingChanged)
    Q_PROPERTY(QString streamStatus READ streamStatus NOTIFY streamLoadingChanged)
    Q_PROPERTY(QString streamKey READ streamKey NOTIFY currentTrackChanged)
    Q_PROPERTY(bool seekable READ seekable NOTIFY seekableChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool shuffle READ shuffle WRITE setShuffle NOTIFY modesChanged)
    Q_PROPERTY(int repeatMode READ repeatMode WRITE setRepeatMode NOTIFY modesChanged)
    Q_PROPERTY(QVariantList queue READ queue NOTIFY queueChanged)
    Q_PROPERTY(QVariantList audioLevels READ audioLevels NOTIFY audioLevelsChanged)
    Q_PROPERTY(double audioEnergy READ audioEnergy NOTIFY audioReactivityChanged)
    Q_PROPERTY(double bassEnergy READ bassEnergy NOTIFY audioReactivityChanged)
    Q_PROPERTY(double beatPulse READ beatPulse NOTIFY audioReactivityChanged)
    Q_PROPERTY(quint64 beatSerial READ beatSerial NOTIFY audioReactivityChanged)
  public:
    PlaybackController(LibraryModel*, SettingsStore*, QObject* parent = nullptr);
    ~PlaybackController() override;
    AudioEqualizer* equalizer()
    {
        return &m_equalizer;
    }
    int currentIndex() const;
    int currentTrackId() const
    {
        return m_currentTrackId;
    }
    QString currentTitle() const
    {
        return m_currentTitle;
    }
    QString currentArtist() const
    {
        return m_currentArtist;
    }
    QString currentFormat() const
    {
        return m_currentFormat;
    }
    QString currentCoverPath() const
    {
        return m_currentCoverPath;
    }
    QUrl artworkUrl() const
    {
        return m_currentCoverPath.startsWith("https://") ? QUrl(m_currentCoverPath)
               : m_currentCoverPath.isEmpty()            ? QUrl()
                                                         : QUrl::fromLocalFile(m_currentCoverPath);
    }
    QColor atmosphereColor() const
    {
        return m_atmosphereColor;
    }
    QColor atmosphereSecondary() const
    {
        return m_atmosphereSecondary;
    }
    double atmosphereShape() const
    {
        return m_atmosphereShape;
    }
    bool favorite() const;
    bool hasTrack() const
    {
        return m_currentTrackId > 0 || m_streaming;
    }
    bool streaming() const
    {
        return m_streaming;
    }
    bool streamLoading() const
    {
        const auto status = m_player.mediaStatus();
        return m_streaming && m_lastError.isEmpty() &&
               (status == QMediaPlayer::LoadingMedia || status == QMediaPlayer::StalledMedia ||
                status == QMediaPlayer::BufferingMedia);
    }
    double streamBufferProgress() const
    {
        return m_streaming ? qBound(0.0, double(m_player.bufferProgress()), 1.0) : 0.0;
    }
    QString streamStatus() const;
    QString streamKey() const
    {
        return m_streaming ? m_streamCandidate.value("key", m_streamCandidate.value("url")).toString() : QString();
    }
    bool seekable() const
    {
        return m_player.isSeekable();
    }
    bool stopped() const
    {
        return m_player.playbackState() == QMediaPlayer::StoppedState;
    }
    bool canPlay() const;
    bool playing() const
    {
        return m_player.playbackState() == QMediaPlayer::PlayingState;
    }
    // Текущая позиция воспроизведения в миллисекундах.
    qint64 position() const
    {
        return m_player.position();
    }
    qint64 duration() const
    {
        return m_player.duration() > 0 ? m_player.duration()
               : m_streaming           ? m_streamCandidate.value("durationMs").toLongLong()
                                       : 0;
    }
    double volume() const
    {
        return m_audioOutput ? m_audioOutput->volume() : m_volume;
    }
    QString lastError() const
    {
        return m_lastError;
    }
    bool shuffle() const
    {
        return m_shuffle;
    }
    int repeatMode() const
    {
        return m_repeat;
    }
    QVariantList queue() const;
    QVariantList audioLevels() const
    {
        return m_audioLevels;
    }
    double audioEnergy() const
    {
        return m_reactivity.state().energy;
    }
    double bassEnergy() const
    {
        return m_reactivity.state().bass;
    }
    double beatPulse() const
    {
        return m_reactivity.state().pulse;
    }
    quint64 beatSerial() const
    {
        return m_reactivity.state().onsets;
    }
    // Начинает воспроизведение трека по его позиции в текущем списке.
    Q_INVOKABLE void playIndex(int index);
    // Начинает воспроизведение трека по его ID.
    Q_INVOKABLE void playTrackId(int id);
    // Воспроизводит онлайн-поток по данным кандидата (URL, заголовки, обложка).
    void playStream(const QVariantMap& candidate);
    // Воспроизводит трек из очереди, сохраняя предыдущий в историю.
    Q_INVOKABLE void playQueueTrack(int id);
    // Запускает подборку рекомендаций («волну»).
    Q_INVOKABLE void playMix();
    // Очередь конкретной подборки; фильтр библиотеки на неё не влияет.
    Q_INVOKABLE void playSelection(const QVariantList& ids, int firstId = 0);
    void playStreamQueue(const QVariantList& tracks, int index = 0);
    // Переключает воспроизведение/паузу.
    Q_INVOKABLE void toggle();
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    // Останавливает воспроизведение и сбрасывает буфер эквалайзера.
    Q_INVOKABLE void stop();
    // Переходит к следующему треку очереди.
    Q_INVOKABLE void next();
    // Возвращается к предыдущему треку (или перематывает к началу).
    Q_INVOKABLE void previous();
    // Перематывает на указанную позицию (в миллисекундах).
    Q_INVOKABLE void seek(qint64 position);
    // Переключает «избранное» для текущего трека.
    Q_INVOKABLE void toggleFavorite();
    // Добавляет трек в очередь, если его там ещё нет.
    Q_INVOKABLE void enqueue(int id);
    Q_INVOKABLE void removeFromQueue(int id);
    void setVolume(double volume);
    // Включает/выключает перемешивание и сохраняет настройку.
    void setShuffle(bool value);
    // Задаёт режим повтора (0 — нет, 1 — вся очередь, 2 — один трек).
    void setRepeatMode(int value);
  signals:
    void streamRequested(const QVariantMap& candidate);
    void streamQueueExhausted();
    void currentTrackChanged();
    void playingChanged();
    void streamLoadingChanged();
    void positionChanged();
    void durationChanged();
    void volumeChanged();
    void lastErrorChanged();
    void modesChanged();
    void queueChanged();
    void audioLevelsChanged();
    void audioReactivityChanged();
    void seeked(qint64 position);
    void seekableChanged();

  private:
    void setError(const QString& error);
    void resetReactivity();
    // Пересчитывает цвета и форму «атмосферы» по обложке или заголовку трека.
    void updateAtmosphere(const QImage& cover = {});
    // Сбрасывает выход обработанного звука и состояние фильтров.
    void flushEqualizedAudio();
    // Направляет обработанный эквалайзером буфер в отдельный аудиовыход.
    void outputEqualizedAudio(const QAudioBuffer& buffer);
    // Перекачивает накопленные данные в аудиовыход порциями по мере готовности.
    void pumpEqualizedAudio();
    // Фиксирует факт прослушивания (завершено или прервано).
    void commitListen(bool completed);
    // Накапливает время прослушивания текущего трека.
    void accountListen();
    // Запускает трек по ID: проверяет файл, настраивает плеер и метаданные.
    bool startTrack(int id, bool preserveQueue = false);
    // Переходит к следующему треку с учётом перемешивания и повтора.
    void advance(bool natural);
    LibraryModel* m_library;
    SettingsStore* m_settings;
    QAudioBufferOutput m_bufferOutput;
    AudioEqualizer m_equalizer;
    std::unique_ptr<QAudioSink> m_equalizedSink;
    QIODevice* m_sinkWriter = nullptr;
    QByteArray m_pendingAudio;
    QTimer m_sinkTimer;
    int m_audioGeneration = 0;
    QMediaPlayer m_player;
    StreamProxy m_streamProxy;
    QNetworkAccessManager m_artNetwork;
    QVariantMap m_streamCandidate;
    bool m_streaming = false;
    QVariantList m_audioLevels;
    QElapsedTimer m_meterClock;
    QElapsedTimer m_bufferClock;
    QTimer m_silenceTimer;
    AudioReactivity m_reactivity;
    std::unique_ptr<QAudioOutput> m_audioOutput;
    double m_volume = 0.75;
    int m_currentTrackId = -1;
    QString m_currentTitle, m_currentArtist, m_currentFormat, m_currentCoverPath, m_lastError;
    QColor m_atmosphereColor{"#a887ef"};
    QColor m_atmosphereSecondary{"#83dbc7"};
    double m_atmosphereShape = 0.5;
    QVector<int> m_queue, m_history;
    QVariantList m_streamQueue;
    int m_streamIndex = -1;
    void requestStream(int index);
    void clearStreamQueue();
    bool m_shuffle = false;
    int m_repeat = 0;
    QElapsedTimer m_elapsed;
    qint64 m_listened = 0;
    bool m_counting = false;
};
