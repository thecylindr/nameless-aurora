#pragma once
// Минимальный клиент Chrome DevTools Protocol поверх WebSocket: рукопожатие,
// кадры WebSocket, отправка команд и приём событий от окна Chromium.
#include <QObject>
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QUrl>
class QTcpSocket;
class QTimer;

class CdpClient : public QObject
{
    Q_OBJECT
  public:
    // Один принятый кадр WebSocket (флаг завершения, opcode и данные).
    struct Frame
    {
        bool fin = true;
        quint8 opcode = 1;
        QByteArray payload;
    };
    explicit CdpClient(QObject* parent = nullptr);
    ~CdpClient() override;
    // Подключается к локальному WebSocket отладки (ws://127.0.0.1).
    virtual void connectToHost(const QUrl& url, int timeoutMs = 10000);
    // Разрывает соединение и завершает ожидающие команды.
    virtual void close();
    // Отправляет CDP-команду и возвращает её id (или -1 без соединения).
    virtual int sendCommand(const QString& method, const QJsonObject& params = {});
    virtual bool isConnected() const
    {
        return m_connected;
    }
    // Формирует HTTP-запрос рукопожатия WebSocket.
    static QByteArray handshakeRequest(const QUrl& url, const QByteArray& key);
    // Проверяет ответ рукопожатия (код 101 и Sec-WebSocket-Accept).
    static bool acceptsHandshake(const QByteArray& header, const QByteArray& key);
    // Упаковывает payload в кадр WebSocket с маскированием.
    static QByteArray packFrame(const QByteArray& payload, quint32 maskKey = 0);
    // Разбирает буфер на законченные кадры WebSocket.
    static QList<Frame> feedData(QByteArray* buffer);
    // Приводит текст ошибки протокола к короткой категории для отчёта.
    static QString errorCategory(const QString& message);
  signals:
    // Получено событие CDP (method + params).
    void eventReceived(const QString& method, const QJsonObject& params);
    // Рукопожатие завершено, соединение установлено.
    void connected();
    // Не удалось подключиться.
    void connectFailed(const QString& message);
    // Получен успешный ответ команды.
    void commandResult(int id, const QJsonObject& result);
    // Команда завершилась ошибкой.
    void commandError(int id, const QString& message, int code = 0);
    // Соединение закрыто.
    void socketClosed();

  private:
    // Упаковывает кадр с заданным opcode.
    static QByteArray packFrame(const QByteArray& payload, quint8 opcode, quint32 maskKey);
    // Обрабатывает входящие данные сокета.
    void onSocketData();
    // Обрабатывает один кадр (данные/close/ping/pong).
    void handleFrame(const Frame& frame);
    // Разбирает JSON-сообщение на событие или ответ команды.
    void dispatchMessage(const QByteArray& payload);
    QTcpSocket* m_socket;
    QHash<int, QTimer*> m_pending;
    QByteArray m_buffer;
    QByteArray m_fragment;
    QUrl m_url;
    int m_nextId = 0;
    int m_generation = 0;
    QByteArray m_handshakeKey;
    bool m_connected = false;
    bool m_handshaken = false;
};
