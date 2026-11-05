#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariantMap>
#include <QVariantList>
#include <QSet>
#include <QJsonObject>

// Поиск и получение аудио с Zaycev.net через публичный API (поиск треков и разрешение ссылки).
class ZaycevCatalog : public QObject
{
    Q_OBJECT
  public:
    explicit ZaycevCatalog(QObject* parent = nullptr);
    // Ищет треки по запросу.
    void search(const QString& query);
    // По хэшу download/streaming получает прямую ссылку на аудио.
    void resolve(const QVariantMap& candidate);
    // Отменяет идущие сетевые запросы.
    void cancel();
    // Разбирает ответ API поиска в список вариантов (с хэшами для resolve).
    static QVariantList parse(const QJsonObject& payload);
  signals:
    void ready(const QVariantList& tracks);
    void resolved(const QVariantMap& track);
    void failed(const QString& message);

  private:
    void request(const QUrl& url, const QVariantMap& candidate = {});
    QNetworkAccessManager m_network;
    QSet<QNetworkReply*> m_replies;
    int m_generation = 0;
};
