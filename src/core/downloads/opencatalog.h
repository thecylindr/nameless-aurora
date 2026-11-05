#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariantList>
#include <QJsonObject>
#include <QSet>

// Поиск аудио в открытом каталоге Internet Archive (archive.org) через публичный API.
class OpenCatalog : public QObject
{
    Q_OBJECT
  public:
    explicit OpenCatalog(QObject* parent = nullptr, QNetworkAccessManager* transport = nullptr);
    // Ищет записи по названию и (опционально) исполнителю.
    void search(const QString& title, const QString& artist);
    // Отменяет все идущие сетевые запросы.
    void cancel();
    // Разбирает ответ archive.org и собирает список mp3-файлов в варианты для загрузки.
    static QVariantList parseFiles(const QJsonObject& payload);
  signals:
    void ready(const QVariantList& tracks);
    void failed(const QString& message);

  private:
    void request(const QUrl& url, bool metadata);
    QNetworkAccessManager* m_network;
    QSet<QNetworkReply*> m_replies;
    QVariantList m_tracks;
    int m_generation = 0;
};
