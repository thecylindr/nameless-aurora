-- Aurora SQLite schema v2. Snapshot of LibraryRepository::migrate().
-- Use only with an empty database; the application migrates existing libraries.
PRAGMA foreign_keys = ON;
PRAGMA journal_mode = WAL;
PRAGMA busy_timeout = 5000;
BEGIN;

CREATE TABLE artists(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE);

CREATE TABLE albums(id INTEGER PRIMARY KEY, title TEXT NOT NULL, artist_id INTEGER NOT NULL REFERENCES artists(id), UNIQUE(title,artist_id));

CREATE TABLE sources(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE);

CREATE TABLE tracks(id INTEGER PRIMARY KEY AUTOINCREMENT, title TEXT NOT NULL, artist_id INTEGER NOT NULL REFERENCES artists(id), album_id INTEGER NOT NULL REFERENCES albums(id), source_id INTEGER NOT NULL REFERENCES sources(id), file_path TEXT UNIQUE, source_url TEXT NOT NULL DEFAULT '', format TEXT NOT NULL DEFAULT '', cover_path TEXT NOT NULL DEFAULT '', duration INTEGER NOT NULL DEFAULT 0 CHECK(duration>=0), bitrate INTEGER NOT NULL DEFAULT 0, rating INTEGER NOT NULL DEFAULT 0 CHECK(rating BETWEEN 0 AND 5), play_count INTEGER NOT NULL DEFAULT 0, last_played INTEGER NOT NULL DEFAULT 0, skip_count INTEGER NOT NULL DEFAULT 0, listen_ms INTEGER NOT NULL DEFAULT 0);

CREATE TABLE playlists(id INTEGER PRIMARY KEY, name TEXT NOT NULL CHECK(length(trim(name))>0), created_at INTEGER NOT NULL);

CREATE TABLE playlist_tracks(playlist_id INTEGER NOT NULL REFERENCES playlists(id) ON DELETE CASCADE, track_id INTEGER NOT NULL REFERENCES tracks(id) ON DELETE CASCADE, position INTEGER NOT NULL, PRIMARY KEY(playlist_id,track_id), UNIQUE(playlist_id,position));

CREATE TABLE listening_events(id INTEGER PRIMARY KEY, track_id INTEGER NOT NULL REFERENCES tracks(id) ON DELETE CASCADE, played_at INTEGER NOT NULL, listened_ms INTEGER NOT NULL CHECK(listened_ms>0), skipped INTEGER NOT NULL CHECK(skipped IN(0,1)));

CREATE INDEX idx_track_artist ON tracks(artist_id);

CREATE INDEX idx_track_album ON tracks(album_id);

CREATE INDEX idx_track_history ON tracks(last_played DESC);

CREATE INDEX idx_playlist_track ON playlist_tracks(track_id);

CREATE INDEX idx_events_track ON listening_events(track_id,played_at);

CREATE VIEW track_details AS SELECT t.id,t.title,a.name AS artist,b.title AS album,t.file_path,t.source_url,s.name AS source,t.format,t.cover_path,t.duration,t.bitrate,t.rating,t.play_count,t.last_played,t.skip_count,t.listen_ms FROM tracks t JOIN artists a ON a.id=t.artist_id JOIN albums b ON b.id=t.album_id JOIN sources s ON s.id=t.source_id;

PRAGMA user_version = 2;
COMMIT;
