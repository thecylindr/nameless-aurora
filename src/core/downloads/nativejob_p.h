#pragma once
#include "../downloadmanager.h"
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProcess>
#include "../data/libraryrepository.h"

// Внутренняя структура задачи загрузки (описывает состояние одной записи очереди).
struct DownloadManager::Job
{
    bool native = false;
    // Метаданные трека (trackId, title, artist, mediaUrl и т. д.) из библиотеки или каталога.
    QVariantMap webTrack;
    QNetworkReply* reply = nullptr;
    QFile* output = nullptr;
    QString nativePath;
    QString nativeFailure;
    QString error;
    int id = 0;
    QString title;
    QString url;
    QString service;
    QString quality;
    bool authorized = false;
    // Флаги однократных повторов: авторизованная сессия и повторный запрос ссылки у сервиса.
    bool sessionRetried = false;
    bool nativeUrlRetried = false;
    QString status = QStringLiteral("queued");
    double progress = 0.0;
    QString tempDir;
    QProcess* process = nullptr;
    bool cancelRequested = false;
    QString stdoutBuffer;
    QString stderrBuffer;
    QString processError;
    bool preparing = false, prepared = false;
    int preparationGeneration = 0;
    Track preparedTrack;
};
