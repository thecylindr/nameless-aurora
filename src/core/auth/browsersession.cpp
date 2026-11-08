#include "browsersession.h"
#include "../web/mediasniffer.h"
#include "cdpclient.h"
#include "webloginmanager.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHostAddress>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTcpServer>

// Реализация BrowserSession: запуск Chromium, CDP-команды, навигация,
// перенос cookies, перехват аудио-ссылок и данных API.

namespace
{
// Определяет, относится ли хост к нужному сервису (с учётом поддоменов).
bool servicePage(const QString& host, const QString& service)
{
    const auto domain = [&](const QString& name) {
        return host == name || host.endsWith('.' + name);
    };
    return WebLoginManager::serviceForHost(host) == service || (service == "hitmo" && domain("hitmos.fm")) ||
           (service == "zaycev" && domain("zaycev.net"));
}
} // namespace

BrowserSession::BrowserSession(QObject* parent, QNetworkAccessManager* transport, CdpClient* protocol,
                               std::function<int()> allocatePort)
    : QObject(parent), m_network(transport ? transport : new QNetworkAccessManager(this)),
      m_client(protocol ? protocol : new CdpClient(this)), m_allocatePort(std::move(allocatePort))
{
    m_poll.setInterval(700);
    m_idle.setSingleShot(true);
    m_idle.setInterval(45000);
    connect(&m_poll, &QTimer::timeout, this, &BrowserSession::poll);
    connect(&m_idle, &QTimer::timeout, this, &BrowserSession::stop);
    connect(m_client, &CdpClient::connected, this, &BrowserSession::initializePage);
    connect(m_client, &CdpClient::commandResult, this, &BrowserSession::onResult);
    connect(m_client, &CdpClient::eventReceived, this, &BrowserSession::onEvent);
    connect(m_client, &CdpClient::commandError, this, [this](int id, const QString& error, int code) {
        if (!m_commands.contains(id))
            return;
        const auto task = m_commands.take(id);
        const auto kind = task.first;
        m_playEvaluations.remove(id);
        if (kind == Navigate || kind == NavigateFallback || kind == NavigateProbe) {
            if (task.second != m_navigation || m_hidden || m_navigationReady)
                return;
            if (kind == NavigateProbe)
                m_navigationProbePending = false;
            m_protocolCode = code;
            m_protocolError = CdpClient::errorCategory(error);
            m_protocolStage = kind == Navigate ? "Page.navigate" : "Runtime.evaluate.navigation";
            if (kind == Navigate && !m_navigationFallback) {
                fallbackNavigation();
            } else if (kind == NavigateFallback && error.contains("Cannot find", Qt::CaseInsensitive) &&
                       m_fallbackAttempts < 3) {
                const int generation = m_generation, navigation = m_navigation;
                QTimer::singleShot(250, this, [this, generation, navigation] {
                    if (generation == m_generation && navigation == m_navigation && !m_stopping && !m_hidden &&
                        !m_navigationReady)
                        fallbackNavigation();
                });
            } else if (kind == NavigateProbe || m_protocolError == "context-changed" ||
                       m_protocolError == "navigation-interrupted") {
                const int generation = m_generation, navigation = m_navigation;
                QTimer::singleShot(350, this, [this, generation, navigation] {
                    probeNavigation(generation, navigation);
                });
            } else if (!m_stopping) {
                fail("Браузер не смог открыть страницу сервиса. Повтори операцию или выбери другой источник.");
            }
            return;
        }
        if (kind == SeedCookie) {
            cookieFinished(task.second, false);
            return;
        }
        if (kind == ApiBody || kind == ApiFetch) {
            const auto url = m_apiCommands.take(id);
            if (kind == ApiFetch)
                emit apiFetched(task.second, url, {}, 0);
            return;
        }
        if (kind == Cookies) {
            m_cookiePending = false;
            return;
        }
        if (kind == WindowShow || kind == WindowHide || kind == Ignore)
            return;
        if (kind == Agent) {
            // Метаданные браузера необязательны: зависший рендерер не должен
            // блокировать отправку первого Page.navigate.
            navigate();
            requestCookies();
            return;
        }
        // Редиректы входа уничтожают прежний JS-контекст. Повторяем на новой
        // странице на следующем тике.
        if (kind == Evaluate && (error.contains("Execution context was destroyed", Qt::CaseInsensitive) ||
                                 error.contains("Cannot find context", Qt::CaseInsensitive) ||
                                 error.contains("Cannot find default execution context", Qt::CaseInsensitive) ||
                                 error.contains("Inspected target navigated", Qt::CaseInsensitive))) {
            emit evaluated(task.second, QVariant());
            return;
        }
        m_protocolStage = kind == ClearCookies                ? "Network.clearBrowserCookies"
                          : kind == Agent || kind == Evaluate ? "Runtime.evaluate"
                                                              : "Page.navigate";
        m_protocolError = CdpClient::errorCategory(error);
        m_protocolCode = code;
        if (!m_stopping)
            fail("Не выполнена команда браузера " + m_protocolStage + " (" + m_protocolError +
                 "). Подробности сохранены в отчёте импорта.");
    });
    connect(m_client, &CdpClient::connectFailed, this, [this](const QString&) {
        m_inFlight = false;
        if (!m_stopping && m_elapsed.isValid() && m_elapsed.elapsed() > 15000)
            fail("Не удалось подключить окно сервиса. Закрой его и повтори вход.");
    });
    connect(m_client, &CdpClient::socketClosed, this, [this] {
        if (!m_stopping && !m_hidden)
            fail("Окно сервиса закрылось. Повтори операцию в Aurora.");
    });
}
BrowserSession::~BrowserSession()
{
    stop();
}
QJsonArray BrowserSession::protocolCookies(const QJsonArray& cookies)
{
    QJsonArray output;
    for (const auto& entry : cookies) {
        const auto value = entry.toObject();
        if (value.value("name").toString().isEmpty() || value.value("domain").toString().isEmpty())
            continue;
        QJsonObject cookie{{"name", value.value("name")},
                           {"value", value.value("value")},
                           {"domain", value.value("domain")},
                           {"path", value.value("path").toString("/")},
                           {"secure", value.value("secure").toBool()},
                           {"httpOnly", value.value("httpOnly").toBool()}};
        // Chromium отклоняет поле Domain у cookies с host-префиксом (__Host-).
        if (value.value("name").toString().startsWith("__Host-")) {
            QString host = value.value("domain").toString();
            if (host.startsWith('.'))
                host.remove(0, 1);
            cookie.remove("domain");
            cookie["url"] = "https://" + host + "/";
            cookie["secure"] = true;
            cookie["path"] = "/";
        }
        const double expiry = value.value("expires").toVariant().toDouble();
        if (expiry > 0 && expiry <= QDateTime::currentSecsSinceEpoch())
            continue;
        if (expiry > 0)
            cookie["expires"] = expiry;
        output.append(cookie);
    }
    return output;
}
QUrl BrowserSession::choosePage(const QJsonArray& pages, const QString& service, int port)
{
    // Берём только локальные ws-цели на нашем порту; приоритет у страницы сервиса.
    QUrl blank;
    for (const auto& entry : pages) {
        const auto page = entry.toObject();
        if (page.value("type").toString() != "page")
            continue;
        const QUrl address(page.value("url").toString()), socket(page.value("webSocketDebuggerUrl").toString());
        if (socket.scheme() != "ws" || (socket.host() != "127.0.0.1" && socket.host() != "localhost") ||
            socket.port() != port)
            continue;
        if (servicePage(address.host(), service))
            return socket;
        if (address.toString() == "about:blank")
            blank = socket;
    }
    return blank;
}
QString BrowserSession::failureText(const QString& error)
{
    if (error.contains("ERR_NAME_NOT_RESOLVED"))
        return "Не удалось найти адрес сервиса. Проверь DNS и соединение.";
    if (error.contains("ERR_INTERNET_DISCONNECTED") || error.contains("ERR_NETWORK_CHANGED"))
        return "Соединение прервано. Проверь сеть и повтори попытку.";
    if (error.contains("ERR_PROXY") || error.contains("ERR_TUNNEL"))
        return "Прокси не подключается к сервису. Проверь настройки сети.";
    if (error.contains("ERR_CERT"))
        return "Не удалось проверить сертификат сайта. Проверь дату и время на "
               "устройстве.";
    if (error.contains("ERR_ABORTED"))
        return {};
    return "Сервис не ответил. Проверь, открывается ли он в обычном браузере.";
}
void BrowserSession::open(const QString& browser, const QString& service, const QUrl& url, const QJsonArray& cookies,
                          bool background, bool replaceCookies)
{
    m_idle.stop();
    // Переиспользуем уже запущенное окно, если параметры не изменились.
    if (m_browser && m_browser->state() != QProcess::NotRunning && m_service == service && m_executable == browser &&
        m_background == background && !replaceCookies && m_initialized && m_client->isConnected()) {
        m_target = url;
        m_hidden = false;
        window(true);
        navigate();
        return;
    }
    stop();
    m_stopping = false;
    m_hidden = false;
    m_service = service;
    m_background = background;
    m_executable = browser;
    m_userAgent.clear();
    m_target = url;
    m_cookies = protocolCookies(cookies);
    m_protocolStage.clear();
    m_protocolError.clear();
    m_protocolCode = 0;
    m_acceptedCookies = {};
    m_seedRejected = 0;
    m_seedPending = 0;
    m_apiHeaders = {};
    m_elapsed.start();
    if (m_allocatePort)
        m_port = m_allocatePort();
    else {
        QTcpServer probe;
        if (probe.listen(QHostAddress::LocalHost, 0))
            m_port = probe.serverPort();
    }
    if (m_port <= 0) {
        fail("Система не разрешила локальное подключение к окну сервиса. Запусти "
             "Aurora в обычной сессии "
             "рабочего стола.");
        return;
    }
    const QString profile = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/service-browser/" +
                            (background ? "local-" : "") + service;
    if (!QDir().mkpath(profile)) {
        fail("Не удалось создать профиль сервиса. Проверь доступ к папке данных "
             "Aurora.");
        return;
    }
    // Если профиль уже содержит cookies и их не просят заменить, ничего не переносим.
    if (!replaceCookies &&
        (QFileInfo::exists(profile + "/Default/Cookies") || QFileInfo::exists(profile + "/Default/Network/Cookies")))
        m_cookies = {};
    QFile::setPermissions(profile, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
    m_browser = new QProcess(this);
    m_browser->setStandardOutputFile(QProcess::nullDevice());
    m_browser->setStandardErrorFile(QProcess::nullDevice());
    connect(m_browser, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart)
            fail("Не удалось запустить браузер сервиса. Выбери другой "
                 "браузер в дополнительных "
                 "способах входа.");
    });
    connect(m_browser, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this](int, QProcess::ExitStatus) {
                if (!m_stopping && !m_hidden)
                    fail("Окно сервиса закрыто. Вернись в Aurora и повтори операцию.");
            });
    // Запускаем Chromium с открытым локальным портом отладки и изолированным профилем.
    QStringList arguments{"--remote-debugging-address=127.0.0.1",
                          "--remote-debugging-port=" + QString::number(m_port),
                          "--user-data-dir=" + profile,
                          "--class=aurora-service",
                          "--no-first-run",
                          "--no-default-browser-check",
                          "--disable-session-crashed-bubble"};
    if (background)
        arguments << "--headless=new" << "about:blank";
    else
        arguments << "--app=about:blank";
    m_browser->setProperty("replaceCookies", replaceCookies);
    m_browser->start(browser, arguments);
    if (!m_browser || m_stopping)
        return;
    m_poll.start();
    QTimer::singleShot(200, this, &BrowserSession::poll);
}
void BrowserSession::poll()
{
    if (m_stopping || !m_browser)
        return;
    if (m_initialized && m_client->isConnected()) {
        if (!m_hidden)
            requestCookies();
        return;
    }
    if (m_elapsed.elapsed() > 15000) {
        fail("Браузер не подключился за 15 секунд. Закрой старое окно сервиса и "
             "повтори вход.");
        return;
    }
    if (m_inFlight)
        return;
    m_inFlight = true;
    const int generation = m_generation;
    // Список целей отладки Chromium отдаётся по HTTP на том же порту.
    QNetworkRequest request(QUrl("http://127.0.0.1:" + QString::number(m_port) + "/json/list"));
    request.setTransferTimeout(2000);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    auto* reply = m_network->get(request);
    connect(reply, &QNetworkReply::readyRead, this, [reply] {
        if (reply->bytesAvailable() > 1024 * 1024)
            reply->abort();
    });
    connect(reply, &QNetworkReply::finished, this, [this, reply, generation] {
        const auto bytes = reply->readAll();
        const bool ok = reply->error() == QNetworkReply::NoError;
        reply->deleteLater();
        if (generation != m_generation || m_stopping)
            return;
        m_inFlight = false;
        if (!ok || bytes.size() > 1024 * 1024)
            return;
        const auto socket = choosePage(QJsonDocument::fromJson(bytes).array(), m_service, m_port);
        if (socket.isEmpty())
            return;
        m_inFlight = true;
        m_client->connectToHost(socket, 3500);
    });
}
// Отправляет команду и запоминает её тип для обработки ответа.
int BrowserSession::command(const QString& method, const QJsonObject& params, Command kind, int requestId)
{
    const int id = m_client->sendCommand(method, params);
    if (id >= 0)
        m_commands[id] = {kind, requestId};
    return id;
}
void BrowserSession::initializePage()
{
    if (m_stopping)
        return;
    command("Page.enable", {}, Ignore);
    command("Network.enable", {}, Ignore);
    // Для сервисов с прямыми ссылками на скачивание запрещаем загрузки, но ловим события.
    if (m_service == "hitmo" || m_service == "zaycev")
        command("Browser.setDownloadBehavior", {{"behavior", "deny"}, {"eventsEnabled", true}}, Ignore);
    if (m_background)
        command("Emulation.setDeviceMetricsOverride",
                {{"width", 1440}, {"height", 1000}, {"deviceScaleFactor", 1}, {"mobile", false}}, Ignore);
    if (m_browser && m_browser->property("replaceCookies").toBool())
        command("Network.clearBrowserCookies", {}, ClearCookies);
    else
        seedCookies();
}
void BrowserSession::seedCookies()
{
    m_protocolStage = "Network.setCookie";
    m_seedPending = m_cookies.size();
    if (!m_seedPending) {
        readAgent();
        return;
    }
    for (int i = 0; i < m_cookies.size(); ++i)
        command("Network.setCookie", m_cookies[i].toObject(), SeedCookie, i);
}
void BrowserSession::cookieFinished(int index, bool accepted)
{
    if (accepted)
        m_acceptedCookies.append(m_cookies[index]);
    else
        ++m_seedRejected;
    if (--m_seedPending > 0)
        return;
    if (!m_cookies.isEmpty() &&
        WebLoginManager::hasSessionCookie(WebLoginManager::decodeCookies(m_cookies), m_service) &&
        !WebLoginManager::hasSessionCookie(WebLoginManager::decodeCookies(m_acceptedCookies), m_service)) {
        m_protocolError = "session-cookie-rejected";
        fail("Браузер отклонил cookies входа (Network.setCookie). Сессия не перенесена; открой отчёт импорта.");
        return;
    }
    readAgent();
}
void BrowserSession::readAgent()
{
    m_initialized = true;
    m_inFlight = false;
    m_protocolStage = "Browser.getVersion";
    // Если метаданные браузера не пришли, продолжаем навигацию по таймауту.
    const int id = command("Browser.getVersion", {}, Agent);
    const int generation = m_generation;
    QTimer::singleShot(1500, this, [this, id, generation] {
        if (generation != m_generation || m_stopping || !m_commands.contains(id))
            return;
        m_commands.remove(id);
        navigate();
        requestCookies();
    });
}
QJsonObject BrowserSession::protocolDiagnostic() const
{
    return {{"stage", m_protocolStage},
            {"error", m_protocolError},
            {"code", m_protocolCode},
            {"navigationFallback", m_navigationFallback},
            {"navigationReady", m_navigationReady},
            {"cookiesAccepted", m_acceptedCookies.size()},
            {"cookiesRejected", m_seedRejected}};
}
void BrowserSession::navigate()
{
    ++m_navigation;
    // Ответ от hide(about:blank) или предыдущего трека не должен прерывать
    // новый импорт; удаление также игнорирует запоздавшие ответы по таймауту.
    for (auto it = m_commands.begin(); it != m_commands.end();) {
        if (it->first == Navigate || it->first == NavigateFallback || it->first == NavigateProbe ||
            it->first == Evaluate || it->first == ApiBody || it->first == ApiFetch) {
            m_playEvaluations.remove(it.key());
            it = m_commands.erase(it);
        } else
            ++it;
    }
    m_navigationFallback = false;
    m_navigationReady = false;
    m_navigationProbePending = false;
    m_fallbackAttempts = 0;
    m_loaded.clear();
    m_playbackConfirmed = false;
    m_mainFrame.clear();
    m_documents.clear();
    m_audioCandidates.clear();
    m_apiRequests.clear();
    m_apiCommands.clear();
    m_protocolStage = "Page.navigate";
    const auto address = m_target.toString(QUrl::FullyEncoded);
    if (!m_target.isValid() ||
        (address != "about:blank" && (m_target.scheme() != "https" || !m_target.userInfo().isEmpty() ||
                                      !servicePage(m_target.host(), m_service)))) {
        fail("Некорректная ссылка на сервис. Вставь HTTPS-ссылку на музыку или плейлист.");
        return;
    }
    m_protocolError.clear();
    m_protocolCode = 0;
    command("Page.navigate", {{"url", address}}, Navigate, m_navigation);
    const int generation = m_generation, navigation = m_navigation;
    // Готовность не зависит от ответа Page.navigate или событий домена Page.
    QTimer::singleShot(1000, this, [this, generation, navigation] {
        probeNavigation(generation, navigation);
    });
    QTimer::singleShot(30000, this, [this, generation, navigation] {
        if (generation != m_generation || navigation != m_navigation || m_stopping || m_hidden || m_navigationReady)
            return;
        m_protocolError = "timeout";
        fail("Страница сервиса не открылась за 30 секунд. Проверь соединение или выбери другой источник.");
    });
}
void BrowserSession::fallbackNavigation()
{
    m_navigationFallback = true;
    ++m_fallbackAttempts;
    // Используем обычную навигацию страницы, когда домен Page отклоняет переход.
    // Ставим переход после результата evaluate, чтобы не потерять его при
    // уничтожении старого контекста исполнения Chromium.
    const auto address = QString::fromUtf8(
        QJsonDocument(QJsonArray{m_target.toString(QUrl::FullyEncoded)}).toJson(QJsonDocument::Compact));
    const QString script = "(()=>{const auroraNavigateTo=" + address +
                           "[0];setTimeout(()=>location.assign(auroraNavigateTo),0);return true;})()";
    command("Runtime.evaluate", {{"expression", script}, {"returnByValue", true}, {"userGesture", true}},
            NavigateFallback, m_navigation);
    const int generation = m_generation, navigation = m_navigation;
    QTimer::singleShot(350, this, [this, generation, navigation] {
        probeNavigation(generation, navigation);
    });
}
void BrowserSession::probeNavigation(int generation, int navigation)
{
    if (generation != m_generation || navigation != m_navigation || m_stopping || m_hidden || m_navigationReady ||
        m_navigationProbePending)
        return;
    m_navigationProbePending = true;
    command("Runtime.evaluate",
            {{"expression", "JSON.stringify({auroraNavigationProbe:true,url:location.href,ready:document.readyState})"},
             {"returnByValue", true}},
            NavigateProbe, navigation);
}
void BrowserSession::navigationReady(const QUrl& url)
{
    if (m_hidden || !servicePage(url.host(), m_service))
        return;
    m_navigationReady = true;
    emit loaded(url, true, {});
}
void BrowserSession::onResult(int id, const QJsonObject& result)
{
    if (!m_commands.contains(id) || m_stopping)
        return;
    const auto task = m_commands.take(id);
    if ((task.first == Navigate || task.first == NavigateFallback || task.first == NavigateProbe) &&
        task.second != m_navigation)
        return;
    switch (task.first) {
    case ClearCookies:
        seedCookies();
        break;
    case SeedCookie:
        cookieFinished(task.second, result.value("success").toBool(true));
        break;
    case ApiBody: {
        const auto url = m_apiCommands.take(id);
        auto bytes = result.value("body").toString().toUtf8();
        if (result.value("base64Encoded").toBool())
            bytes = QByteArray::fromBase64(bytes);
        if (bytes.size() > 10 * 1024 * 1024)
            break;
        const auto document = QJsonDocument::fromJson(bytes);
        if (!document.isNull())
            emit apiReceived(url, document.isArray() ? QJsonValue(document.array()) : QJsonValue(document.object()));
        break;
    }
    case ApiFetch: {
        const auto url = m_apiCommands.take(id);
        const auto bytes = result.value("result").toObject().value("value").toString().toUtf8();
        const auto response = QJsonDocument::fromJson(bytes).object();
        emit apiFetched(task.second, url, response.value("payload"), response.value("status").toInt());
        break;
    }
    case Cookies:
        m_cookiePending = false;
        emit cookiesReceived(m_service, result.value("cookies").toArray());
        break;
    case Agent:
        m_userAgent = result.value("userAgent").toString();
        // Скрываем признак headless-браузера, чтобы сервис не отклонял сессию.
        if (m_background && !m_userAgent.isEmpty()) {
            m_userAgent.replace("HeadlessChrome/", "Chrome/");
            command("Network.setUserAgentOverride", {{"userAgent", m_userAgent}, {"acceptLanguage", "ru-RU,ru,en"}},
                    Ignore);
        }
        navigate();
        requestCookies();
        break;
    case Evaluate:
        if (result.contains("exceptionDetails")) {
            fail("Сайт не разрешил прочитать список треков. Открой сам плейлист и "
                 "повтори импорт.");
            return;
        }
        {
            const bool playing = m_playEvaluations.remove(id) > 0;
            const auto value = result.value("result").toObject().value("value").toVariant();
            if (playing && QJsonDocument::fromJson(value.toString().toUtf8()).object().value("activated").toBool())
                m_playbackConfirmed = true;
            emit evaluated(task.second, value);
            if (playing && !m_hidden && !m_stopping && m_audioCandidates.size() == 1 &&
                QJsonDocument::fromJson(value.toString().toUtf8()).object().value("activated").toBool())
                emit audioRequested(QUrl(*m_audioCandidates.cbegin()));
        }
        break;
    case Navigate:
        m_mainFrame = result.value("frameId").toString();
        if (result.contains("errorText")) {
            const auto text = failureText(result.value("errorText").toString());
            if (result.value("errorText").toString().contains("ERR_ABORTED") && !m_hidden && !m_navigationReady &&
                !m_navigationFallback)
                fallbackNavigation();
            else if (!text.isEmpty() && !m_hidden && !m_navigationReady)
                emit loaded(m_target, false, text);
        }
        break;
    case NavigateFallback:
        if (result.contains("exceptionDetails") && !m_hidden && !m_navigationReady) {
            m_protocolStage = "Runtime.evaluate.navigation";
            m_protocolError = "navigation-script-failed";
            fail("Не удалось перейти на страницу сервиса. Повтори операцию или выбери другой источник.");
        }
        break;
    case NavigateProbe: {
        m_navigationProbePending = false;
        const auto value =
            QJsonDocument::fromJson(result.value("result").toObject().value("value").toString().toUtf8()).object();
        const QUrl url(value.value("url").toString());
        // Готовый документ предыдущего трека ещё не означает, что запрошенная
        // навигация завершилась; обычные редиректы обрабатываются отдельно.
        const bool requestedPage =
            url.path() == m_target.path() ||
            (m_service == "vk" && m_target.path() == "/audio" && url.path().startsWith("/audios")) ||
            (m_target.path() == "/collection" && url.path().startsWith("/collection/"));
        if (!result.contains("exceptionDetails") && requestedPage &&
            QStringList{"interactive", "complete"}.contains(value.value("ready").toString()) &&
            servicePage(url.host(), m_service)) {
            m_loaded = url;
            navigationReady(url);
        } else {
            const int generation = m_generation, navigation = m_navigation;
            QTimer::singleShot(500, this, [this, generation, navigation] {
                probeNavigation(generation, navigation);
            });
        }
        break;
    }
    case WindowShow:
    case WindowHide:
        if (result.contains("windowId"))
            command("Browser.setWindowBounds",
                    {{"windowId", result.value("windowId")},
                     {"bounds", QJsonObject{{"windowState", task.first == WindowShow ? "normal" : "minimized"}}}},
                    Ignore);
        break;
    default:
        break;
    }
}
void BrowserSession::onEvent(const QString& method, const QJsonObject& params)
{
    if (m_stopping)
        return;
    if (method == "Page.frameNavigated") {
        const auto frame = params.value("frame").toObject();
        if (frame.contains("parentId"))
            return;
        m_loaded = QUrl(frame.value("url").toString());
        m_mainFrame = frame.value("id").toString();
    } else if (method == "Page.domContentEventFired" || method == "Page.loadEventFired") {
        if (!m_loaded.isEmpty())
            navigationReady(m_loaded);
        requestCookies();
    } else if (method == "Network.requestWillBeSent" && !m_hidden) {
        const auto request = params.value("request").toObject();
        const QUrl apiUrl(request.value("url").toString());
        if (m_service == "yandex" && apiUrl.scheme() == "https" && apiUrl.host() == "api.music.yandex.ru") {
            if ((isCollectionApi(apiUrl) || isWaveApi(apiUrl)) && m_apiRequests.size() < 128)
                m_apiRequests[params.value("requestId").toString()] = apiUrl;
            const auto headers = request.value("headers").toObject();
            for (auto it = headers.begin(); it != headers.end(); ++it)
                if (QStringList{"authorization", "x-yandex-music-client", "x-yandex-music-device"}.contains(
                        it.key().toLower()))
                    m_apiHeaders[it.key()] = it.value(); // Только на время сессии; не сохраняется и не попадает в отчёты.
        }
        if (params.value("type").toString() == "Document" &&
            (params.value("frameId").toString() == m_mainFrame ||
             params.value("request").toObject().value("url").toString() == m_target.toString()))
            m_documents.insert(params.value("requestId").toString());
        const QUrl url(params.value("request").toObject().value("url").toString());
        if (servicePage(m_loaded.host(), m_service) && MediaSniffer::isAudioUrl(url)) {
            if (m_audioCandidates.size() < 16)
                m_audioCandidates.insert(url.toString());
            if (m_service != "vk" || m_playbackConfirmed)
                emit audioRequested(url);
        }
    } else if (method == "Network.responseReceived" && !m_hidden && servicePage(m_loaded.host(), m_service)) {
        const auto response = params.value("response").toObject();
        const QUrl url(response.value("url").toString());
        const auto mime = response.value("mimeType").toString().toLower();
        if (url.scheme() == "https" && url.userInfo().isEmpty() && mime.startsWith("audio/")) {
            if (m_audioCandidates.size() < 16)
                m_audioCandidates.insert(url.toString());
            if (m_service != "vk" || m_playbackConfirmed)
                emit audioRequested(url);
        }
    } else if (method == "Browser.downloadWillBegin" && servicePage(m_loaded.host(), m_service)) {
        const QUrl url(params.value("url").toString());
        if (MediaSniffer::isAudioUrl(url))
            emit audioRequested(url);
    } else if (method == "Network.loadingFinished" && m_apiRequests.contains(params.value("requestId").toString())) {
        const auto requestId = params.value("requestId").toString();
        const auto url = m_apiRequests.take(requestId);
        if (params.value("encodedDataLength").toDouble() <= 10 * 1024 * 1024) {
            const int id = command("Network.getResponseBody", {{"requestId", requestId}}, ApiBody);
            if (id >= 0)
                m_apiCommands[id] = url;
        }
    } else if (method == "Network.loadingFailed" && m_documents.contains(params.value("requestId").toString())) {
        const auto text = failureText(params.value("errorText").toString());
        if (!text.isEmpty() && !m_hidden)
            emit loaded(m_target, false, text);
    } else if (method == "Inspector.detached" || method == "Inspector.targetCrashed") {
        if (!m_hidden)
            fail("Браузер остановил страницу сервиса. Закрой окно и повтори "
                 "операцию.");
    }
}
bool BrowserSession::isCollectionApi(const QUrl& url)
{
    if (url.scheme() != "https" || url.host() != "api.music.yandex.ru" || !url.userInfo().isEmpty() ||
        (url.port() != -1 && url.port() != 443))
        return false;
    return url.path() == "/tracks" || url.path() == "/landing-blocks/collection/playlist-with-likes" ||
           QRegularExpression("^/playlist/[A-Za-z0-9_.-]{1,100}$").match(url.path()).hasMatch();
}
void BrowserSession::fetchJson(int requestId, const QUrl& url, const QJsonArray& trackIds)
{
    // Выполняем fetch в контексте страницы, подставляя сохранённые заголовки API.
    if (m_stopping || !m_initialized || (!isCollectionApi(url) && !isWaveApi(url)) || m_service != "yandex" ||
        WebLoginManager::serviceForHost(m_loaded.host()) != "yandex" || trackIds.size() > 200) {
        emit apiFetched(requestId, url, {}, 0);
        return;
    }
    const auto args = QString::fromUtf8(
        QJsonDocument(QJsonObject{{"url", url.toString()}, {"headers", m_apiHeaders}, {"ids", trackIds}})
            .toJson(QJsonDocument::Compact));
    const QString script = "(async()=>{const a=" + args +
                           ";const c=new AbortController();const timer=setTimeout(()=>c.abort(),12000);"
                           "try{const o={credentials:'include',headers:a.headers,signal:c.signal};"
                           "if(a.ids.length){o.method='POST';o.body=new FormData();for(const id of "
                           "a.ids)o.body.append('trackIds',id);o.body.append('removeDuplicates','false');}"
                           "const r=await fetch(a.url,o);const text=await r.text();if(text.length>10485760)return "
                           "JSON.stringify({status:413});"
                           "return JSON.stringify({status:r.status,payload:JSON.parse(text)});}catch(e){return "
                           "JSON.stringify({status:0});}finally{clearTimeout(timer);}})()";
    const int id =
        command("Runtime.evaluate", {{"expression", script}, {"returnByValue", true}, {"awaitPromise", true}}, ApiFetch,
                requestId);
    if (id >= 0)
        m_apiCommands[id] = url;
}
bool BrowserSession::isWaveApi(const QUrl& url)
{
    return url.scheme() == "https" && url.host() == "api.music.yandex.ru" && url.userInfo().isEmpty()
           && (url.port() == -1 || url.port() == 443)
           && url.path() == "/rotor/station/user:onyourwave/tracks";
}
void BrowserSession::evaluate(int requestId, const QString& script, bool userGesture)
{
    if (!m_initialized || !m_client->isConnected()) {
        fail("Окно сервиса ещё не подключено. Повтори операцию.");
        return;
    }
    const int id =
        command("Runtime.evaluate",
                {{"expression", script}, {"returnByValue", true}, {"awaitPromise", true}, {"userGesture", userGesture}},
                Evaluate, requestId);
    if (userGesture && id >= 0)
        m_playEvaluations.insert(id);
}
void BrowserSession::requestCookies()
{
    if (m_stopping || !m_initialized || m_cookiePending || !m_client->isConnected())
        return;
    m_cookiePending = true;
    command("Storage.getCookies", {}, Cookies);
}
void BrowserSession::window(bool visible)
{
    if (!m_background && m_client->isConnected())
        command("Browser.getWindowForTarget", {}, visible ? WindowShow : WindowHide);
}
void BrowserSession::hide()
{
    if (!m_browser)
        return;
    requestCookies();
    m_hidden = true;
    window(false);
    // Останавливаем воспроизведение и отрисовку сайта между операциями;
    // подключённый процесс держим недолго для пакетной работы.
    m_target = QUrl("about:blank");
    if (m_initialized)
        navigate();
    m_idle.start();
}
void BrowserSession::fail(const QString& error)
{
    stop();
    emit failed(error);
}
void BrowserSession::stop()
{
    m_stopping = true;
    ++m_generation;
    m_poll.stop();
    m_idle.stop();
    m_commands.clear();
    m_playEvaluations.clear();
    m_apiRequests.clear();
    m_apiCommands.clear();
    m_apiHeaders = {};
    m_initialized = false;
    m_inFlight = false;
    m_cookiePending = false;
    m_client->close();
    for (auto* reply : m_network->findChildren<QNetworkReply*>()) {
        disconnect(reply, nullptr, this, nullptr);
        reply->abort();
        reply->deleteLater();
    }
    if (m_browser) {
        disconnect(m_browser, nullptr, this, nullptr);
        m_browser->terminate();
        if (!m_browser->waitForFinished(1200)) {
            m_browser->kill();
            m_browser->waitForFinished(500);
        }
        m_browser->deleteLater();
        m_browser = nullptr;
    }
    m_port = 0;
}
