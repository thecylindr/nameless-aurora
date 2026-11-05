#include "oauthmanager.h"
#include "../settingsstore.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QUrlQuery>

// Реализация OAuthManager: локальный сервер обратного вызова, обмен кода
// на токен и синхронное обновление токена Яндекса.

namespace
{
// Фиксированный порт локального OAuth callback и соответствующий redirect URI.
const quint16 callbackPort = 18765;
const QString redirectUri = QStringLiteral("http://127.0.0.1:18765/");
// Процентное кодирование строки.
QString percent(const QString& value)
{
    return QString::fromLatin1(QUrl::toPercentEncoding(value));
}
} // namespace

OAuthManager::OAuthManager(SettingsStore* settings, QObject* parent) : QObject(parent), m_settings(settings)
{}

bool OAuthManager::vkLoggedIn() const
{
    return m_settings && !m_settings->vkToken().isEmpty();
}
bool OAuthManager::yandexLoggedIn() const
{
    return m_settings && !m_settings->yandexToken().isEmpty();
}
QString OAuthManager::vkToken() const
{
    return m_settings ? m_settings->vkToken() : QString();
}

QString OAuthManager::vkAuthorizeUrl(const QString& clientId)
{
    return QStringLiteral("https://oauth.vk.com/"
                          "authorize?client_id=%1&redirect_uri=http%3A%2F%2F127.0.0.1%3A18765%2F&response_type=code&"
                          "scope=audio&state=aurora&v=5.199")
        .arg(percent(clientId));
}
QString OAuthManager::yandexAuthorizeUrl(const QString& clientId)
{
    return QStringLiteral("https://oauth.yandex.ru/"
                          "authorize?response_type=code&client_id=%1&redirect_uri=http%3A%2F%2F127.0.0.1%3A18765%2F")
        .arg(percent(clientId));
}

bool OAuthManager::parseRedirectQuery(const QString& raw, QString* code, QString* accessToken, qint64* expiresIn,
                                      QString* state, QString* error)
{
    code->clear();
    accessToken->clear();
    state->clear();
    *expiresIn = 0;
    error->clear();
    int start = raw.indexOf('?');
    if (start < 0)
        start = raw.indexOf('#');
    if (start < 0 || start + 1 >= raw.size())
        return false;
    const QUrlQuery query(raw.mid(start + 1));
    const QString errorCode = query.queryItemValue(QStringLiteral("error"));
    const QString description = query.queryItemValue(QStringLiteral("error_description"));
    if (!errorCode.isEmpty()) {
        *error = description.isEmpty() ? errorCode : errorCode + QStringLiteral(": ") + description;
        return false;
    }
    *code = query.queryItemValue(QStringLiteral("code"));
    *accessToken = query.queryItemValue(QStringLiteral("access_token"));
    if (query.hasQueryItem(QStringLiteral("expires_in")))
        *expiresIn = query.queryItemValue(QStringLiteral("expires_in")).toLongLong();
    *state = query.queryItemValue(QStringLiteral("state"));
    return !code->isEmpty() || !accessToken->isEmpty();
}

bool OAuthManager::applyTokenJson(SettingsStore* settings, const QString& service, const QByteArray& json,
                                  QString* message)
{
    message->clear();
    QJsonParseError parse;
    const auto document = QJsonDocument::fromJson(json, &parse);
    if (parse.error != QJsonParseError::NoError || !document.isObject()) {
        *message = QStringLiteral("Сервис вернул некорректный ответ.");
        return false;
    }
    const auto root = document.object();
    const auto errorValue = root.value(QStringLiteral("error"));
    QString errorText;
    if (errorValue.isString())
        errorText = errorValue.toString();
    else if (errorValue.isObject())
        errorText = errorValue.toObject().value(QStringLiteral("error_msg")).toString();
    const QString description = root.value(QStringLiteral("error_description")).toString();
    if (!errorText.isEmpty()) {
        *message = description.isEmpty() ? errorText : errorText + QStringLiteral(": ") + description;
        return false;
    }
    const QString accessToken = root.value(QStringLiteral("access_token")).toString();
    if (accessToken.isEmpty()) {
        *message = QStringLiteral("Сервис не вернул токен.");
        return false;
    }
    const bool vk = service == QStringLiteral("VK");
    if (vk)
        settings->setVkToken(accessToken);
    else {
        settings->setYandexToken(accessToken);
        const QString refreshToken = root.value(QStringLiteral("refresh_token")).toString();
        if (!refreshToken.isEmpty())
            settings->setYandexRefreshToken(refreshToken);
        const qint64 expiresIn = qRound64(root.value(QStringLiteral("expires_in")).toDouble());
        settings->setYandexTokenExpiry(expiresIn <= 0 ? 0 : QDateTime::currentSecsSinceEpoch() + expiresIn);
    }
    *message = vk ? QStringLiteral("VK подключён.") : QStringLiteral("Яндекс Музыка подключена.");
    return true;
}

void OAuthManager::begin(const QString& service)
{
    cleanup();
    m_service = service;
    m_busy = true;
    emit busyChanged();
    m_lastError.clear();
    emit lastErrorChanged();
}
void OAuthManager::cleanup()
{
    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }
    if (m_server) {
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }
    if (m_busy) {
        m_busy = false;
        emit busyChanged();
    }
}
void OAuthManager::fail(const QString& service, const QString& message)
{
    cleanup();
    m_lastError = message;
    emit lastErrorChanged();
    emit loginDone(service, false, message);
}
void OAuthManager::succeed(const QString& service, const QString& message)
{
    cleanup();
    emit accountsChanged();
    emit loginDone(service, true, message);
}

void OAuthManager::login(const QString& service)
{
    if (m_busy || !m_settings)
        return;
    const bool vk = service == QStringLiteral("VK");
    const QString clientId = vk ? m_settings->vkClientId() : m_settings->yandexClientId();
    const QString clientSecret = vk ? m_settings->vkClientSecret() : m_settings->yandexClientSecret();
    if (clientId.isEmpty()) {
        m_lastError = QStringLiteral("Укажите Client ID приложения %1.").arg(service);
        emit lastErrorChanged();
        emit loginDone(service, false, m_lastError);
        return;
    }
    if (vk && clientSecret.isEmpty()) {
        m_lastError = QStringLiteral("Для VK нужен и защищённый ключ (client_secret) приложения.");
        emit lastErrorChanged();
        emit loginDone(service, false, m_lastError);
        return;
    }
    // Локальный сервер принимает redirect от браузера после авторизации.
    auto* server = new QTcpServer(this);
    if (!server->listen(QHostAddress(QStringLiteral("127.0.0.1")), callbackPort)) {
        server->deleteLater();
        m_lastError = QStringLiteral("Не удалось открыть порт %1 — закройте другие приложения, которые его занимают.")
                          .arg(callbackPort);
        emit lastErrorChanged();
        emit loginDone(service, false, m_lastError);
        return;
    }
    begin(service);
    m_server = server;
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, [this] {
        fail(m_service, QStringLiteral("Время ожидания входа истекло. Попробуйте ещё раз."));
    });
    m_timer->start(150000);
    const QUrl url(vk ? vkAuthorizeUrl(clientId) : yandexAuthorizeUrl(clientId));
    if (!QDesktopServices::openUrl(url)) {
        fail(m_service, QStringLiteral("Не удалось открыть браузер."));
        return;
    }
    connect(server, &QTcpServer::newConnection, this, [this] {
        auto* socket = m_server ? m_server->nextPendingConnection() : nullptr;
        if (!socket)
            return;
        socket->setParent(this);
        // Принимаем запросы только с локального адреса.
        if (socket->peerAddress() != QHostAddress(QStringLiteral("127.0.0.1"))) {
            socket->disconnectFromHost();
            return;
        }
        connect(socket, &QTcpSocket::readyRead, this, [this, socket] {
            while (socket->canReadLine()) {
                const QByteArray line = socket->readLine();
                if (line.startsWith("GET ") || line.startsWith("POST ")) {
                    // Показываем пользователю страницу «Вход выполнен» в браузере.
                    static const QByteArray page = QByteArrayLiteral(
                        "<!doctype html><html><head><meta charset=\"utf-8\"><title>Aurora Player</title></head>"
                        "<body "
                        "style=\"margin:0;background:#0b0e1a;color:#eaf2ff;font-family:sans-serif;display:flex;align-"
                        "items:center;justify-content:center;height:100vh\">"
                        "<div style=\"text-align:center\"><h2 style=\"font-weight:600\">Вход выполнен</h2><p "
                        "style=\"color:#9aa4c8\">Можно вернуться в Aurora Player</p></div>"
                        "</body></html>");
                    socket->write("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: " +
                                  QByteArray::number(page.size()) + "\r\nConnection: close\r\n\r\n" + page);
                    socket->flush();
                    const QString target = QString::fromLatin1(line).section(' ', 1, 1);
                    handleCallback(target);
                    socket->disconnectFromHost();
                    return;
                }
                if (line == "\r\n" || line == "\n")
                    break;
            }
        });
    });
}

void OAuthManager::handleCallback(const QString& target)
{
    const QString service = m_service;
    QString code, accessToken, state, error;
    qint64 expiresIn = 0;
    parseRedirectQuery(target, &code, &accessToken, &expiresIn, &state, &error);
    if (!error.isEmpty()) {
        fail(service, error);
        return;
    }
    if (service == QStringLiteral("VK") && !state.isEmpty() && state != QStringLiteral("aurora")) {
        fail(service, QStringLiteral("Проверка state не прошла — попробуйте войти заново."));
        return;
    }
    if (!accessToken.isEmpty()) {
        if (service == QStringLiteral("VK")) {
            m_settings->setVkToken(accessToken);
            succeed(service, QStringLiteral("VK подключён."));
        } else {
            m_settings->setYandexToken(accessToken);
            m_settings->setYandexTokenExpiry(expiresIn <= 0 ? 0 : QDateTime::currentSecsSinceEpoch() + expiresIn);
            succeed(service, QStringLiteral("Яндекс Музыка подключена."));
        }
        return;
    }
    if (code.isEmpty()) {
        fail(service, QStringLiteral("Сервис не вернул код авторизации."));
        return;
    }
    exchangeCode(service, code);
}

void OAuthManager::exchangeCode(const QString& service, const QString& code)
{
    QNetworkRequest request;
    QByteArray body;
    // VK принимает GET с query-параметрами, Яндекс — POST с form-encoded телом.
    if (service == QStringLiteral("VK")) {
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("client_id"), m_settings->vkClientId());
        query.addQueryItem(QStringLiteral("client_secret"), m_settings->vkClientSecret());
        query.addQueryItem(QStringLiteral("code"), code);
        query.addQueryItem(QStringLiteral("redirect_uri"), redirectUri);
        request.setUrl(QUrl(QStringLiteral("https://oauth.vk.com/access_token?") + query.query(QUrl::FullyEncoded)));
    } else {
        QUrlQuery form;
        form.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("authorization_code"));
        form.addQueryItem(QStringLiteral("code"), code);
        form.addQueryItem(QStringLiteral("client_id"), m_settings->yandexClientId());
        form.addQueryItem(QStringLiteral("client_secret"), m_settings->yandexClientSecret());
        body = form.query(QUrl::FullyEncoded).toUtf8();
        request.setUrl(QUrl(QStringLiteral("https://oauth.yandex.ru/token")));
        request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    }
    request.setTransferTimeout(30000);
    auto* reply = service == QStringLiteral("VK") ? m_network.get(request) : m_network.post(request, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply, service] {
        const auto bytes = reply->readAll();
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            fail(service, QStringLiteral("Не удалось обменять код на токен: %1").arg(reply->errorString()));
            return;
        }
        QString message;
        if (!applyTokenJson(m_settings, service, bytes, &message)) {
            fail(service, message);
            return;
        }
        succeed(service, message);
    });
}

void OAuthManager::logout(const QString& service)
{
    if (!m_settings)
        return;
    if (service == QStringLiteral("VK"))
        m_settings->setVkToken({});
    else {
        m_settings->setYandexToken({});
        m_settings->setYandexRefreshToken({});
        m_settings->setYandexTokenExpiry(0);
    }
    emit accountsChanged();
}

void OAuthManager::saveToken(const QString& service, const QString& token)
{
    const QString cleaned = token.trimmed();
    if (cleaned.isEmpty()) {
        m_lastError = QStringLiteral("Вставьте токен.");
        emit lastErrorChanged();
        emit loginDone(service, false, m_lastError);
        return;
    }
    if (!m_settings)
        return;
    if (service == QStringLiteral("VK"))
        m_settings->setVkToken(cleaned);
    else {
        m_settings->setYandexToken(cleaned);
        m_settings->setYandexTokenExpiry(0);
    }
    m_lastError.clear();
    emit lastErrorChanged();
    emit accountsChanged();
    emit loginDone(service, true,
                   service == QStringLiteral("VK") ? QStringLiteral("Токен VK сохранён.")
                                                   : QStringLiteral("Токен Яндекс Музыки сохранён."));
}

QString OAuthManager::serviceHint(const QString& service) const
{
    if (service == QStringLiteral("VK"))
        return QStringLiteral("1. Откройте vk.com/dev и создайте приложение типа «Standalone».\n"
                              "2. В разделе «Настройки» скопируйте ID приложения и защищённый ключ в поля ниже.\n"
                              "3. Доступ к аудио (scope audio) VK выдаёт только верифицированным приложениям — при "
                              "отказе API импортируйте плейлист TXT/CSV/JSON файлом.\n"
                              "Редирект при регистрации не требуется.");
    return QStringLiteral("1. Откройте oauth.yandex.ru и зарегистрируйте приложение типа «Веб-сервисы».\n"
                          "2. Укажите Callback URI: http://127.0.0.1:18765/ (он же Redirect URI).\n"
                          "3. Скопируйте ClientID и Client secret в поля ниже.");
}

QString OAuthManager::yandexToken()
{
    if (!m_settings)
        return {};
    QString token = m_settings->yandexToken();
    if (token.isEmpty())
        return {};
    const qint64 expiry = m_settings->yandexTokenExpiry();
    if (expiry > 0 && expiry <= QDateTime::currentSecsSinceEpoch()) {
        if (m_settings->yandexRefreshToken().isEmpty()) {
            m_settings->setYandexToken({});
            emit accountsChanged();
            return {};
        }
        if (!refreshYandexToken())
            return {};
        token = m_settings->yandexToken();
    }
    return token;
}

bool OAuthManager::refreshYandexToken()
{
    if (!m_settings)
        return false;
    const QString refresh = m_settings->yandexRefreshToken();
    if (refresh.isEmpty())
        return false;
    QUrlQuery form;
    form.addQueryItem(QStringLiteral("grant_type"), QStringLiteral("refresh_token"));
    form.addQueryItem(QStringLiteral("refresh_token"), refresh);
    form.addQueryItem(QStringLiteral("client_id"), m_settings->yandexClientId());
    form.addQueryItem(QStringLiteral("client_secret"), m_settings->yandexClientSecret());
    QNetworkRequest request(QUrl(QStringLiteral("https://oauth.yandex.ru/token")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));
    request.setTransferTimeout(10000);
    auto* reply = m_network.post(request, form.query(QUrl::FullyEncoded).toUtf8());
    // Синхронно ждём ответа (до 10 с) через локальный event loop.
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(10000);
    if (!reply->isFinished())
        loop.exec();
    bool ok = false;
    QString message;
    if (reply->isFinished() && reply->error() == QNetworkReply::NoError)
        ok = applyTokenJson(m_settings, QStringLiteral("Яндекс Музыка"), reply->readAll(), &message);
    reply->deleteLater();
    if (!ok) {
        m_settings->setYandexToken({});
        m_settings->setYandexRefreshToken({});
        m_settings->setYandexTokenExpiry(0);
    }
    emit accountsChanged();
    return ok;
}
