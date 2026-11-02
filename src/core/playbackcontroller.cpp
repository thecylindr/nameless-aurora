#include "playbackcontroller.h"

#include "librarymodel.h"
#include "settingsstore.h"

#include <QAudioOutput>
#include <QUrl>

PlaybackController::PlaybackController(LibraryModel *library, SettingsStore *settings, QObject *parent)
    : QObject(parent), m_library(library), m_settings(settings) {
    m_player.setAudioOutput(&m_audioOutput);
    m_audioOutput.setVolume(static_cast<float>(m_settings->volume()));
    connect(&m_player, &QMediaPlayer::positionChanged, this, &PlaybackController::positionChanged);
    connect(&m_player, &QMediaPlayer::durationChanged, this, &PlaybackController::durationChanged);
    connect(&m_player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState) {
        emit playingChanged();
    });
    connect(&m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            commitListen(true);
            next();
        }
    });
    connect(&m_player, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error, const QString &error) { setError(error); });
    connect(m_settings, &SettingsStore::volumeChanged, this, [this] {
        m_audioOutput.setVolume(static_cast<float>(m_settings->volume()));
        emit volumeChanged();
    });
}

PlaybackController::~PlaybackController() {
    commitListen(false);
}

void PlaybackController::playIndex(int index) {
    const QModelIndex modelIndex = m_library->index(index, 0);
    if (!modelIndex.isValid()) {
        setError(QStringLiteral("Трек не найден в текущем списке."));
        return;
    }
    commitListen(false);
    const QString path = m_library->data(modelIndex, LibraryModel::FilePathRole).toString();
    m_currentIndex = index;
    m_currentTrackId = m_library->data(modelIndex, LibraryModel::IdRole).toInt();
    m_currentTitle = m_library->data(modelIndex, LibraryModel::TitleRole).toString();
    m_currentArtist = m_library->data(modelIndex, LibraryModel::ArtistRole).toString();
    m_currentFormat = m_library->data(modelIndex, LibraryModel::FormatRole).toString();
    m_currentCoverPath = m_library->data(modelIndex, LibraryModel::CoverPathRole).toString();
    setError({});
    m_player.setSource(QUrl::fromLocalFile(path));
    m_player.play();
    m_library->incrementPlayCount(m_currentTrackId);
    emit currentTrackChanged();
}

void PlaybackController::playTrackId(int id) {
    int row = m_library->rowForTrackId(id);
    if (row < 0) {
        m_library->setSearchText({});
        row = m_library->rowForTrackId(id);
    }
    if (row >= 0)
        playIndex(row);
    else
        setError(QStringLiteral("Трек больше не найден в библиотеке."));
}

void PlaybackController::commitListen(bool completed) {
    if (m_currentTrackId <= 0)
        return;
    const qint64 listened = qBound<qint64>(0, position(), duration() > 0 ? duration() : position());
    const bool skipped = !completed && listened > 5000 && duration() > 0 && listened < duration() * 0.8;
    m_library->recordListen(m_currentTrackId, listened, skipped);
    m_currentTrackId = -1;
}

void PlaybackController::toggle() {
    if (!hasTrack()) {
        if (m_library->rowCount() > 0)
            playIndex(0);
        return;
    }
    if (m_player.playbackState() == QMediaPlayer::PlayingState)
        m_player.pause();
    else
        m_player.play();
}

void PlaybackController::next() {
    if (m_library->rowCount() == 0)
        return;
    playIndex((m_currentIndex + 1 + m_library->rowCount()) % m_library->rowCount());
}

void PlaybackController::previous() {
    if (position() > 3000) {
        seek(0);
        return;
    }
    if (m_library->rowCount() == 0)
        return;
    playIndex((m_currentIndex - 1 + m_library->rowCount()) % m_library->rowCount());
}

void PlaybackController::seek(qint64 position) {
    m_player.setPosition(qMax<qint64>(0, position));
}

void PlaybackController::setVolume(double volume) {
    m_settings->setVolume(volume);
}

void PlaybackController::setError(const QString &error) {
    if (m_lastError == error)
        return;
    m_lastError = error;
    emit lastErrorChanged();
}
