#include "browsercookieimporter.h"
#include "../settingsstore.h"
#include "webloginmanager.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <algorithm>

// Реализация импорта cookies: поиск профилей, запуск Python-скрипта,
// разбор ответа и перебор кандидатов до найденной сессии.

BrowserCookieImporter::BrowserCookieImporter(SettingsStore* settings, QObject* parent, const QString& profileHome)
    : QObject(parent), m_settings(settings), m_profileHome(profileHome)
{}
BrowserCookieImporter::~BrowserCookieImporter()
{
    cancel();
}

QVariantList BrowserCookieImporter::profiles(const QString& home, const QString& configRoot)
{
    const QString config = configRoot.isEmpty() ? home + "/.config" : configRoot;
    QVariantList found;
    // Добавляет профиль, если найдена его база cookies, без дубликатов по каноническому пути.
    auto add = [&](const QString& browser, const QString& label, const QString& path, const QString& name,
                   bool preferred) {
        const QFileInfo dir(path);
        const QFileInfo db(browser == "firefox"                           ? path + "/cookies.sqlite"
                           : QFileInfo::exists(path + "/Network/Cookies") ? path + "/Network/Cookies"
                                                                          : path + "/Cookies");
        if (!db.isFile())
            return;
        const QString canonical = dir.canonicalFilePath();
        for (const auto& entry : found)
            if (entry.toMap().value("id").toString() == canonical)
                return;
        found.append(QVariantMap{{"id", canonical},
                                 {"browser", browser},
                                 {"path", canonical},
                                 {"label", label + " · " + name},
                                 {"preferred", preferred},
                                 {"modified", db.lastModified().toMSecsSinceEpoch()}});
    };
    // Известные Chromium-подобные браузеры и пути к их профилям (включая Flatpak/Snap).
    const QList<QStringList> chromium{
        {"brave", "Brave", config + "/BraveSoftware/Brave-Browser"},
        {"chrome", "Chrome", config + "/google-chrome"},
        {"chromium", "Chromium", config + "/chromium"},
        {"yandex", "Яндекс Браузер", config + "/yandex-browser"},
        {"edge", "Edge", config + "/microsoft-edge"},
        {"vivaldi", "Vivaldi", config + "/vivaldi"},
        {"opera", "Opera", config + "/opera"},
        {"brave", "Brave (Flatpak)", home + "/.var/app/com.brave.Browser/config/BraveSoftware/Brave-Browser"},
        {"chrome", "Chrome (Flatpak)", home + "/.var/app/com.google.Chrome/config/google-chrome"},
        {"chromium", "Chromium (Flatpak)", home + "/.var/app/org.chromium.Chromium/config/chromium"},
        {"chromium", "Chromium (Snap)", home + "/snap/chromium/common/chromium"},
    };
    for (const auto& browser : chromium) {
        QFile state(browser[2] + "/Local State");
        QJsonObject profile;
        if (state.open(QIODevice::ReadOnly) && state.size() < 16 * 1024 * 1024)
            profile = QJsonDocument::fromJson(state.readAll()).object().value("profile").toObject();
        const auto names = profile.value("info_cache").toObject();
        for (const auto& entry : QDir(browser[2]).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            const auto key = entry.fileName();
            if (key != "Default" && !key.startsWith("Profile "))
                continue;
            add(browser[0], browser[1], entry.absoluteFilePath(),
                names.value(key).toObject().value("name").toString(key),
                key == profile.value("last_used").toString("Default"));
        }
        if (browser[0] == "opera")
            add(browser[0], browser[1], browser[2], "Default", true);
    }
    // Firefox хранит профили в profiles.ini, путь может быть относительным.
    for (const auto& root : QStringList{config + "/mozilla/firefox", home + "/.mozilla/firefox",
                                        home + "/snap/firefox/common/.mozilla/firefox",
                                        home + "/.var/app/org.mozilla.firefox/.mozilla/firefox"}) {
        QSettings ini(root + "/profiles.ini", QSettings::IniFormat);
        for (const auto& group : ini.childGroups()) {
            if (!group.startsWith("Profile"))
                continue;
            ini.beginGroup(group);
            const QString entry = ini.value("Path").toString();
            const bool relative = ini.value("IsRelative", 1).toBool();
            if (!entry.isEmpty())
                add("firefox", "Firefox", relative ? root + "/" + entry : entry,
                    ini.value("Name", QFileInfo(entry).fileName()).toString(), ini.value("Default", 0).toBool());
            ini.endGroup();
        }
    }
    // Сначала «предпочтительные» профили, затем по свежести изменения базы cookies.
    std::stable_sort(found.begin(), found.end(), [](const QVariant& a, const QVariant& b) {
        const auto x = a.toMap(), y = b.toMap();
        if (x.value("preferred").toBool() != y.value("preferred").toBool())
            return x.value("preferred").toBool();
        return x.value("modified").toLongLong() > y.value("modified").toLongLong();
    });
    return found;
}
QVariantList BrowserCookieImporter::availableProfiles() const
{
    return m_profileHome.isEmpty() ? profiles(QDir::homePath(), qEnvironmentVariable("XDG_CONFIG_HOME"))
                                   : profiles(m_profileHome);
}
QString BrowserCookieImporter::python() const
{
    const QString managed =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/tools/venv/bin/python3";
    if (QFileInfo(managed).isExecutable())
        return managed;
    return QStandardPaths::findExecutable("python3");
}
QString BrowserCookieImporter::errorText(const QString& code)
{
    if (code == "keyring")
        return "Cookies найдены, но системное хранилище ключей не дало их "
               "расшифровать. Разблокируй keyring в текущей сессии рабочего стола "
               "и повтори импорт.";
    if (code == "dependency")
        return "Для чтения зашифрованных cookies нужен модуль yt-dlp. Нажми "
               "«Обновить» в разделе загрузчиков.";
    if (code == "locked")
        return "Не удалось прочитать базу cookies. Проверь доступ к профилю; если "
               "он заблокирован браузером, закрой браузер и повтори.";
    if (code == "timeout")
        return "Чтение сессии браузера заняло слишком долго. Проверь доступ к "
               "профилю и системному keyring.";
    if (code == "missing")
        return "В выбранном профиле не найдена база cookies.";
    if (code == "no_session")
        return "В выбранных профилях нет действующей сессии этого сервиса. Выбери "
               "профиль, в котором сервис уже открыт под твоим аккаунтом.";
    return "Не удалось прочитать сессию браузера. Выбери другой профиль или "
           "проверь доступ к его базе cookies.";
}
void BrowserCookieImporter::read(const QString& service, const QString& profileId)
{
    cancel();
    m_service = WebLoginManager::normalizeService(service);
    m_error.clear();
    m_explicit = !profileId.isEmpty();
    if (m_service.isEmpty()) {
        complete({}, "Неизвестный музыкальный сервис.");
        return;
    }
    m_candidates = availableProfiles();
    if (m_explicit) {
        QVariantList selected;
        for (const auto& value : m_candidates)
            if (value.toMap().value("id").toString() == profileId)
                selected.append(value);
        m_candidates = selected;
    } else if (m_settings) {
        // Без явного выбора ставим профили предпочитаемого браузера вперёд.
        const auto preferred = m_settings->browser();
        std::stable_partition(m_candidates.begin(), m_candidates.end(), [&](const QVariant& v) {
            return v.toMap().value("browser").toString() == preferred;
        });
    }
    if (m_candidates.isEmpty()) {
        complete({}, "Профили браузеров с cookies не найдены. Проверь выбранный "
                     "профиль в настройках импорта.");
        return;
    }
    next();
}
void BrowserCookieImporter::next()
{
    if (m_candidates.isEmpty()) {
        complete({}, errorText(m_error.isEmpty() ? "no_session" : m_error));
        return;
    }
    m_current = m_candidates.takeFirst().toMap();
    emit statusChanged("Читаем сессию " + m_current.value("label").toString() + "…");
    QFile script(":/native/scripts/browser_cookies.py");
    const auto program = python();
    if (program.isEmpty() || !script.open(QIODevice::ReadOnly)) {
        complete({}, "Для чтения сессии нужен Python 3 и компоненты загрузчиков.");
        return;
    }
    // Запускаем Python-скрипт, который достаёт cookies из базы браузера.
    auto* process = new QProcess(this);
    m_process = process;
    m_output.clear();
    process->setStandardErrorFile(QProcess::nullDevice());
    connect(process, &QProcess::readyReadStandardOutput, this, [this, process] {
        m_output += process->readAllStandardOutput();
        if (m_output.size() > 4 * 1024 * 1024)
            process->kill();
    });
    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart || process != m_process)
            return;
        m_process = nullptr;
        process->deleteLater();
        complete({}, "Не удалось запустить чтение сессии браузера.");
    });
    // Ограничиваем чтение 20 секундами — расшифровка может зависнуть на keyring.
    auto* timeout = new QTimer(process);
    timeout->setSingleShot(true);
    timeout->setInterval(20000);
    connect(timeout, &QTimer::timeout, process, &QProcess::kill);
    timeout->start();
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this, process, timeout](int code, QProcess::ExitStatus status) {
                if (process != m_process)
                    return;
                m_output += process->readAllStandardOutput();
                m_process = nullptr;
                process->deleteLater();
                const auto root = QJsonDocument::fromJson(m_output).object();
                m_output.fill('\0');
                m_output.clear();
                QString error = root.value("error").toString();
                if (error.isEmpty() && (code != 0 || status != QProcess::NormalExit))
                    error = timeout->isActive() ? "read" : "timeout";
                const auto cookies = root.value("cookies").toArray();
                if (error.isEmpty() &&
                    WebLoginManager::hasSessionCookie(WebLoginManager::decodeCookies(cookies), m_service)) {
                    complete(cookies);
                    return;
                }
                if (error.isEmpty())
                    error = "no_session";
                if (m_error.isEmpty() || error != "no_session")
                    m_error = error;
                // Явно выбранный профиль или нехватка зависимости — останавливаемся сразу.
                if (m_explicit || error == "dependency") {
                    complete({}, errorText(error));
                    return;
                }
                next();
            });
    process->start(program, {"-c", QString::fromUtf8(script.readAll()), m_current.value("browser").toString(),
                             m_current.value("path").toString(), m_service});
}
void BrowserCookieImporter::complete(const QJsonArray& cookies, const QString& error)
{
    m_candidates.clear();
    emit finished(m_service, m_current.value("id").toString(), cookies, error);
}
void BrowserCookieImporter::cancel()
{
    m_candidates.clear();
    if (m_process) {
        disconnect(m_process, nullptr, this, nullptr);
        m_process->kill();
        m_process->waitForFinished(1000);
        m_process->deleteLater();
        m_process = nullptr;
    }
    m_output.fill('\0');
    m_output.clear();
}
