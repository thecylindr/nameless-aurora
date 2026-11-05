#pragma once
#include <QObject>
#include <QTcpServer>
#include <QNetworkAccessManager>
#include <QSet>
#include <QUrl>
#include <QVariantMap>
class QTcpSocket;
class QNetworkReply;

// Ограниченный, поддерживающий Range-запросы ретранслятор потока в памяти для HTTP-читателя QMediaPlayer.
// Пробрасывает заголовки выбранного потока, не записывая медиаданные на диск.
class StreamProxy : public QObject
{
    Q_OBJECT
  public:
    explicit StreamProxy(QObject* parent = nullptr);
    ~StreamProxy() override;
    // Запускает локальный сервер-прокси и возвращает URL для воспроизведения (или пустой URL при ошибке).
    QUrl open(const QUrl& remote, const QVariantMap& headers, const QString& referer);
    // Останавливает сервер и разрывает все активные соединения.
    void reset();

  private:
    void accept(QTcpSocket* socket);
    QTcpServer m_server;
    QNetworkAccessManager m_network;
    QSet<QTcpSocket*> m_clients;
    QSet<QNetworkReply*> m_replies;
    QUrl m_remote;
    QVariantMap m_headers;
    QString m_path, m_referer;
};
