#include "cdpclient.h"

#include <QJsonDocument>
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QTcpSocket>
#include <QTimer>

// Реализация клиента CDP: рукопожатие WebSocket, кадры, диспетчеризация
// сообщений и таймауты ожидания ответов команд.

CdpClient::CdpClient(QObject* parent) : QObject(parent), m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, [this] {
        QByteArray entropy;
        for (int i = 0; i < 4; ++i)
            entropy += QByteArray::number(QRandomGenerator::global()->generate(), 16);
        m_handshakeKey = QCryptographicHash::hash(entropy, QCryptographicHash::Md5).toBase64();
        m_socket->write(handshakeRequest(m_url, m_handshakeKey));
    });
    connect(m_socket, &QTcpSocket::readyRead, this, &CdpClient::onSocketData);
    connect(m_socket, &QTcpSocket::errorOccurred, this, [this] {
        if (!m_connected)
            emit connectFailed(m_socket->errorString());
        else {
            m_connected = false;
            emit socketClosed();
        }
    });
}
CdpClient::~CdpClient()
{
    for (auto* timer : std::as_const(m_pending))
        timer->deleteLater();
    m_pending.clear();
}

QByteArray CdpClient::packFrame(const QByteArray& payload, quint32 maskKey)
{
    return packFrame(payload, 1, maskKey);
}
QByteArray CdpClient::packFrame(const QByteArray& payload, quint8 opcode, quint32 maskKey)
{
    // Клиент обязан маскировать исходящие кадры (RFC 6455).
    if (maskKey == 0)
        maskKey = QRandomGenerator::global()->generate();
    QByteArray frame;
    frame.append(char(0x80 | (opcode & 0x0f)));
    const quint64 size = quint64(payload.size());
    if (size < 126)
        frame.append(char(0x80 | quint8(size)));
    else if (size <= 0xffff) {
        frame.append(char(0x80 | 126));
        frame.append(char(size >> 8));
        frame.append(char(size & 0xff));
    } else {
        frame.append(char(0x80 | 127));
        for (int shift = 56; shift >= 0; shift -= 8)
            frame.append(char((size >> shift) & 0xff));
    }
    frame.append(char(maskKey >> 24));
    frame.append(char(maskKey >> 16));
    frame.append(char(maskKey >> 8));
    frame.append(char(maskKey));
    for (qsizetype i = 0; i < payload.size(); ++i)
        frame.append(char(payload.at(i) ^ char(maskKey >> ((3 - (i % 4)) * 8))));
    return frame;
}

QList<CdpClient::Frame> CdpClient::feedData(QByteArray* buffer)
{
    // Разбираем заголовок кадра (FIN/opcode, длину, маску) и извлекаем payload.
    QList<Frame> frames;
    forever
    {
        if (buffer->size() < 2)
            break;
        const quint8 b0 = quint8(buffer->at(0)), b1 = quint8(buffer->at(1));
        quint64 length = b1 & 0x7f;
        int offset = 2;
        if (length == 126) {
            if (buffer->size() < 4)
                break;
            length = (quint64(quint8(buffer->at(2))) << 8) | quint8(buffer->at(3));
            offset = 4;
        } else if (length == 127) {
            if (buffer->size() < 10)
                break;
            length = 0;
            for (int i = 0; i < 8; ++i)
                length = (length << 8) | quint8(buffer->at(2 + i));
            offset = 10;
        }
        const bool masked = b1 & 0x80;
        quint32 maskKey = 0;
        if (masked) {
            if (buffer->size() < offset + 4)
                break;
            maskKey = (quint32(quint8(buffer->at(offset))) << 24) | (quint32(quint8(buffer->at(offset + 1))) << 16) |
                      (quint32(quint8(buffer->at(offset + 2))) << 8) | quint8(buffer->at(offset + 3));
            offset += 4;
        }
        if (length > 20 * 1024 * 1024) {
            buffer->clear();
            return frames;
        }
        if (buffer->size() < offset + qint64(length))
            break;
        Frame frame;
        frame.fin = b0 & 0x80;
        frame.opcode = b0 & 0x0f;
        frame.payload = buffer->mid(offset, int(length));
        if (masked)
            for (qsizetype i = 0; i < frame.payload.size(); ++i)
                frame.payload[i] = char(frame.payload.at(i) ^ char(maskKey >> ((3 - (i % 4)) * 8)));
        *buffer = buffer->mid(offset + int(length));
        frames.append(frame);
    }
    return frames;
}

void CdpClient::connectToHost(const QUrl& url, int timeoutMs)
{
    close();
    m_buffer.clear();
    m_fragment.clear();
    m_url = url;
    m_handshaken = false;
    m_connected = false;
    if (url.scheme() != "ws" || (url.host() != "127.0.0.1" && url.host() != "localhost")) {
        emit connectFailed("Допустим только локальный WebSocket браузера.");
        return;
    }
    m_socket->connectToHost(url.host(), quint16(url.port(80)));
    // Поколение обрезает устаревший таймаут после повторного подключения.
    const int generation = ++m_generation;
    QTimer::singleShot(timeoutMs, this, [this, generation] {
        if (generation != m_generation)
            return;
        if (!m_connected && !m_handshaken) {
            m_socket->abort();
            emit connectFailed(QStringLiteral("Таймаут подключения."));
        }
    });
}

void CdpClient::onSocketData()
{
    m_buffer += m_socket->readAll();
    if (m_buffer.size() > 21 * 1024 * 1024) {
        m_socket->abort();
        emit connectFailed("Слишком большой ответ браузера.");
        return;
    }
    // Сначала ждём HTTP-ответ рукопожатия, затем разбираем кадры.
    if (!m_handshaken) {
        const int end = m_buffer.indexOf("\r\n\r\n");
        if (end < 0)
            return;
        const QByteArray header = m_buffer.left(end);
        m_buffer = m_buffer.mid(end + 4);
        if (!header.startsWith("HTTP/1.1 101") && !header.startsWith("HTTP/1.0 101")) {
            emit connectFailed(QString::fromLatin1(header.left(header.indexOf('\r'))));
            m_socket->disconnectFromHost();
            return;
        }
        if (!acceptsHandshake(header, m_handshakeKey)) {
            emit connectFailed("Браузер отклонил WebSocket handshake.");
            m_socket->abort();
            return;
        }
        m_handshaken = true;
        m_connected = true;
        emit connected();
    }
    const auto frames = feedData(&m_buffer);
    for (const auto& frame : frames)
        handleFrame(frame);
}

void CdpClient::handleFrame(const Frame& frame)
{
    // 0x0 — продолжение, 0x1 — текст, 0x2 — бинарные данные.
    switch (frame.opcode) {
    case 0x0:
    case 0x1:
    case 0x2:
        if (!frame.fin) {
            m_fragment += frame.payload;
            if (m_fragment.size() > 20 * 1024 * 1024) {
                m_fragment.clear();
                m_socket->abort();
            }
            return;
        }
        if (frame.opcode == 0 && !m_fragment.isEmpty()) {
            dispatchMessage(m_fragment + frame.payload);
            m_fragment.clear();
        } else if (frame.opcode == 0x1)
            dispatchMessage(frame.payload);
        break;
    case 0x9: // ping — отвечаем pong (0xA).
        if (m_socket->isOpen())
            m_socket->write(packFrame(frame.payload, 0xA, 0));
        break;
    case 0x8: // close — закрываем соединение.
        m_connected = false;
        emit socketClosed();
        m_socket->disconnectFromHost();
        break;
    default:
        break;
    }
}

void CdpClient::dispatchMessage(const QByteArray& payload)
{
    QJsonParseError parse;
    const auto document = QJsonDocument::fromJson(payload, &parse);
    if (parse.error != QJsonParseError::NoError || !document.isObject())
        return;
    const auto root = document.object();
    if (!root.contains(QStringLiteral("id"))) {
        if (root.value("method").isString())
            emit eventReceived(root.value("method").toString(), root.value("params").toObject());
        return;
    }
    const int id = root.value(QStringLiteral("id")).toInt();
    auto* timer = m_pending.take(id);
    if (timer) {
        timer->stop();
        timer->deleteLater();
    }
    if (root.value(QStringLiteral("error")).isObject()) {
        const auto error = root.value(QStringLiteral("error")).toObject();
        emit commandError(id, error.value(QStringLiteral("message")).toString(), error.value("code").toInt());
        return;
    }
    emit commandResult(id, root.value(QStringLiteral("result")).toObject());
}

int CdpClient::sendCommand(const QString& method, const QJsonObject& params)
{
    if (!m_connected)
        return -1;
    const int id = ++m_nextId;
    const QByteArray payload =
        QJsonDocument(QJsonObject{{"id", id}, {"method", method}, {"params", params}}).toJson(QJsonDocument::Compact);
    m_socket->write(packFrame(payload, 1, 0));
    // Каждая команда получает таймаут, чтобы не зависать без ответа.
    auto* timer = new QTimer(this);
    timer->setSingleShot(true);
    m_pending.insert(id, timer);
    connect(timer, &QTimer::timeout, this, [this, id, timer] {
        if (!m_pending.remove(id))
            return;
        timer->deleteLater();
        emit commandError(id, QStringLiteral("Command timed out"));
    });
    timer->start(15000);
    return id;
}

QString CdpClient::errorCategory(const QString& message)
{
    // На диск попадают только эти фиксированные метки; текст протокола может содержать приватные URL.
    const auto text = message.toLower();
    if (text.contains("timeout") || text.contains("timed out") || text.contains("таймаут"))
        return "timeout";
    if (text.contains("invalid url") || text.contains("invalidurl"))
        return "invalid-url";
    if (text.contains("invalid") || text.contains("deserialize"))
        return "invalid-parameters";
    if (text.contains("not attached") || text.contains("target closed") || text.contains("session with given id"))
        return "target-unavailable";
    if (text.contains("frame") &&
        (text.contains("not found") || text.contains("detached") || text.contains("too early")))
        return "frame-unavailable";
    if (text.contains("context") && (text.contains("destroyed") || text.contains("cannot find")))
        return "context-changed";
    if (text.contains("err_aborted") || text.contains("cancelled") || text.contains("canceled"))
        return "navigation-interrupted";
    if (text.contains("forbidden") || text.contains("not allowed"))
        return "navigation-denied";
    if (text.contains("crash"))
        return "renderer-crashed";
    return "command-rejected";
}

void CdpClient::close()
{
    ++m_generation;
    for (auto* timer : std::as_const(m_pending))
        timer->deleteLater();
    m_pending.clear();
    // Вежливо отправляем кадр закрытия (0x8), затем обрываем сокет.
    if (m_socket->isOpen() && m_connected)
        m_socket->write(packFrame({}, 0x8, 0));
    m_socket->abort();
    m_connected = false;
}

QByteArray CdpClient::handshakeRequest(const QUrl& url, const QByteArray& key)
{
    // Ключ Sec-WebSocket-Key нужен для проверки ответа сервера.
    QByteArray target = url.path(QUrl::FullyEncoded).toUtf8();
    if (target.isEmpty())
        target = "/";
    if (!url.query().isEmpty())
        target += '?' + url.query(QUrl::FullyEncoded).toUtf8();
    return "GET " + target + " HTTP/1.1\r\nHost: " + url.host().toUtf8() + ":" + QByteArray::number(url.port(80)) +
           "\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: " + key +
           "\r\nSec-WebSocket-Version: 13\r\n\r\n";
}
bool CdpClient::acceptsHandshake(const QByteArray& header, const QByteArray& key)
{
    if (!header.startsWith("HTTP/1.1 101 ") && !header.startsWith("HTTP/1.0 101 "))
        return false;
    // Sec-WebSocket-Accept = SHA1(key + фиксированный GUID) в Base64.
    const auto expected =
        QCryptographicHash::hash(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", QCryptographicHash::Sha1).toBase64();
    QByteArray accepted, upgrade, connection;
    for (auto line : header.split('\n')) {
        const auto name = line.left(line.indexOf(':')).trimmed().toLower();
        const auto value = line.mid(line.indexOf(':') + 1).trimmed();
        if (name == "sec-websocket-accept")
            accepted = value;
        if (name == "upgrade")
            upgrade = value.toLower();
        if (name == "connection")
            connection = value.toLower();
    }
    bool connectionUpgrade = false;
    for (const auto& token : connection.split(','))
        if (token.trimmed() == "upgrade")
            connectionUpgrade = true;
    return accepted == expected && upgrade == "websocket" && connectionUpgrade;
}
