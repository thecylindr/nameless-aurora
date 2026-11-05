#include "streamproxy.h"
#include <QTcpSocket>
#include <QNetworkReply>
#include <QPointer>
#include <QTimer>
#include <QUuid>
#include <memory>

StreamProxy::StreamProxy(QObject* parent) : QObject(parent), m_network(this)
{
    connect(&m_server, &QTcpServer::newConnection, this, [this] {
        while (m_server.hasPendingConnections()) {
            auto* socket = m_server.nextPendingConnection();
            // Ограничиваем число одновременных клиентов, чтобы не исчерпать ресурсы.
            if (m_clients.size() >= 4) {
                socket->abort();
                socket->deleteLater();
                continue;
            }
            m_clients.insert(socket);
            accept(socket);
        }
    });
}
StreamProxy::~StreamProxy()
{
    reset();
}
void StreamProxy::reset()
{
    m_server.close();
    const auto replies = m_replies;
    m_replies.clear();
    for (auto* reply : replies) {
        disconnect(reply, nullptr, nullptr, nullptr);
        reply->abort();
        reply->deleteLater();
    }
    const auto clients = m_clients;
    m_clients.clear();
    for (auto* socket : clients) {
        socket->abort();
        socket->deleteLater();
    }
}
QUrl StreamProxy::open(const QUrl& remote, const QVariantMap& headers, const QString& referer)
{
    reset();
    if (remote.scheme() != "https" || remote.host().isEmpty() || !remote.userInfo().isEmpty())
        return {};
    m_remote = remote;
    m_headers = headers;
    m_referer = referer;
    // HLS-плейлисты (.m3u8) отдаём напрямую — QMediaPlayer сам разберёт вложенные сегменты.
    if (remote.path().endsWith(".m3u8", Qt::CaseInsensitive))
        return remote;
    if (!m_server.listen(QHostAddress::LocalHost, 0))
        return {};
    // Случайный путь затрудняет доступ к прокси извне процесса.
    m_path = "/" + QUuid::createUuid().toString(QUuid::WithoutBraces) + "/audio";
    return QUrl("http://127.0.0.1:" + QString::number(m_server.serverPort()) + m_path);
}
void StreamProxy::accept(QTcpSocket* socket)
{
    auto request = std::make_shared<QByteArray>();
    auto started = std::make_shared<bool>(false);
    connect(socket, &QTcpSocket::disconnected, this, [this, socket] {
        m_clients.remove(socket);
        socket->deleteLater();
    });
    // Если клиент не прислал HTTP-запрос за 5 секунд, разрываем соединение.
    QTimer::singleShot(5000, socket, [socket, started] {
        if (!*started)
            socket->abort();
    });
    connect(socket, &QTcpSocket::readyRead, socket, [this, socket, request, started] {
        if (*started) {
            socket->readAll();
            return;
        }
        request->append(socket->readAll());
        // Ограничиваем размер заголовка, защищаясь от злонамеренного клиента.
        if (request->size() > 8192) {
            socket->abort();
            return;
        }
        // Ждём завершения HTTP-заголовков (пустая строка после них).
        if (!request->contains("\r\n\r\n"))
            return;
        *started = true;
        const auto lines = request->split('\n');
        const auto first = lines.first().trimmed().split(' ');
        // Принимаем только GET/HEAD по нашему секретному пути.
        if (first.size() < 3 || (first[0] != "GET" && first[0] != "HEAD") || first[1] != m_path.toUtf8()) {
            socket->write("HTTP/1.0 404 Not Found\r\nContent-Length: 0\r\n\r\n");
            socket->disconnectFromHost();
            return;
        }
        QNetworkRequest upstream(m_remote);
        upstream.setTransferTimeout(30000);
        upstream.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        upstream.setMaximumRedirectsAllowed(4);
        // Просим сервер не сжимать поток — проще отдавать байты клиенту как есть.
        upstream.setRawHeader("Accept-Encoding", "identity");
        for (auto i = m_headers.cbegin(); i != m_headers.cend(); ++i) {
            const auto key = i.key().toLower();
            const auto value = i.value().toString();
            // Пробрасываем только безопасные заголовки, отбрасывая переводы строк и длинные значения.
            if (QStringList{"user-agent", "referer", "origin"}.contains(key) && !value.contains('\r') &&
                !value.contains('\n'))
                upstream.setRawHeader(key.toUtf8(), value.left(1500).toUtf8());
        }
        if (!upstream.hasRawHeader("User-Agent"))
            upstream.setRawHeader("User-Agent", "Mozilla/5.0 AuroraPlayer/4.0");
        if (!upstream.hasRawHeader("Referer")) {
            const QUrl u(m_referer);
            // Referer нужен сервисам для проверки источника запроса.
            if (u.scheme() == "https")
                upstream.setRawHeader("Referer", u.adjusted(QUrl::RemoveFragment | QUrl::RemoveUserInfo).toEncoded());
        }
        // Перенаправляем Range-заголовок клиента — это позволяет QMediaPlayer перематывать поток.
        for (const auto& line : lines)
            if (line.toLower().startsWith("range:")) {
                const auto value = line.mid(6).trimmed();
                if (value.startsWith("bytes=") && value.size() < 100)
                    upstream.setRawHeader("Range", value);
            }
        auto* reply = first[0] == "HEAD" ? m_network.head(upstream) : m_network.get(upstream);
        m_replies.insert(reply);
        reply->setReadBufferSize(512 * 1024);
        const QPointer<QNetworkReply> safeReply(reply);
        auto sent = std::make_shared<bool>(false);
        const auto pump = [socket, safeReply, sent] {
            if (!safeReply || socket->state() != QAbstractSocket::ConnectedState)
                return;
            auto* r = safeReply.data();
            const int status = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            // Ждём финального ответа после редиректов, прежде чем отдавать заголовки клиенту.
            if (status >= 300 && status < 400 && !r->isFinished())
                return;
            if (!*sent && (status > 0 || r->isFinished())) {
                const int code = status > 0 ? status : 502;
                QByteArray head = "HTTP/1.0 " + QByteArray::number(code) +
                                  (code == 206   ? " Partial Content\r\n"
                                   : code == 200 ? " OK\r\n"
                                                 : " Source Error\r\n");
                // Отдаём клиенту только нужные заголовки, без служебных полей прокси.
                for (const auto& key : QList<QByteArray>{"Content-Type", "Content-Length", "Content-Range",
                                                         "Accept-Ranges", "Content-Encoding"})
                    if (r->hasRawHeader(key)) {
                        const auto value = r->rawHeader(key);
                        if (!value.contains('\r') && !value.contains('\n'))
                            head += key + ": " + value + "\r\n";
                    }
                socket->write(head + "Connection: close\r\nCache-Control: no-store\r\n\r\n");
                *sent = true;
            }
            if (!*sent)
                return;
            // Перекачиваем доступные байты, не давая буферу сокета разрастись.
            const auto room = 256 * 1024 - socket->bytesToWrite();
            if (room > 0 && r->bytesAvailable() > 0)
                socket->write(r->read(room));
            if (r->isFinished() && !r->bytesAvailable())
                socket->disconnectFromHost();
        };
        connect(reply, &QNetworkReply::metaDataChanged, socket, pump);
        connect(reply, &QNetworkReply::readyRead, socket, pump);
        connect(reply, &QNetworkReply::finished, socket, pump);
        connect(socket, &QTcpSocket::bytesWritten, socket, [pump](qint64) {
            pump();
        });
        connect(socket, &QTcpSocket::disconnected, reply, [this, reply] {
            m_replies.remove(reply);
            reply->abort();
            reply->deleteLater();
        });
        pump();
    });
}
