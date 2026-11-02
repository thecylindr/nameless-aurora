#pragma once

#include <QAbstractListModel>
#include <QVector>

struct Track {
    int id = 0;
    QString title;
    QString artist;
    QString album;
    QString filePath;
    QString sourceUrl;
    QString source;
    QString format;
    QString coverPath;
    qint64 duration = 0;
    qint64 lastPlayed = 0;
    qint64 listenMs = 0;
    int bitrate = 0;
    int rating = 0;
    int playCount = 0;
    int skipCount = 0;
};

class LibraryModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        ArtistRole,
        AlbumRole,
        FilePathRole,
        SourceUrlRole,
        SourceRole,
        FormatRole,
        CoverPathRole,
        DurationRole,
        BitrateRole,
        RatingRole,
        PlayCountRole,
        LastPlayedRole,
        SkipCountRole,
        ListenMsRole
    };
    Q_ENUM(Role)

    explicit LibraryModel(QObject *parent = nullptr);
    ~LibraryModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_tracks.size(); }
    QString searchText() const { return m_searchText; }
    QString lastError() const { return m_lastError; }

    void setSearchText(const QString &text);
    bool addTrack(const QString &filePath, const QString &sourceUrl = {}, const QString &source = {});
    void setRating(int id, int rating);
    void removeTrack(int id);
    void incrementPlayCount(int id);
    void recordListen(int id, qint64 milliseconds, bool skipped);
    int rowForTrackId(int id) const;
    Q_INVOKABLE QVariantList recommendations(int limit = 12) const;
    void refresh();

signals:
    void countChanged();
    void searchTextChanged();
    void lastErrorChanged();
    void libraryChanged();

private:
    void setError(const QString &error);
    bool createSchema();
    Track probeFile(const QString &filePath) const;
    QString extractCover(const QString &filePath, int trackId) const;

    QVector<Track> m_tracks;
    QString m_searchText;
    QString m_lastError;
    QString m_connectionName;
};
