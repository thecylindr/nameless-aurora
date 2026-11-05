#include "namelessbridge.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

namespace
{
// Читает JSON-файл и возвращает его объект (пустой объект при ошибке).
QJsonObject readJson(const QString& path)
{
    QFile f(path);
    return f.open(QIODevice::ReadOnly) ? QJsonDocument::fromJson(f.readAll()).object() : QJsonObject();
}
} // namespace
NamelessBridge::NamelessBridge(QObject* parent) : QObject(parent)
{
    // Перечитываем тему с небольшой задержкой после изменений, объединяя серию событий файловой системы.
    m_debounce.setSingleShot(true);
    m_debounce.setInterval(150);
    connect(&m_debounce, &QTimer::timeout, this, &NamelessBridge::reload);
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this] {
        m_debounce.start();
    });
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this] {
        m_debounce.start();
    });
    reload();
}
void NamelessBridge::reload()
{
    // Пути хранения тем Nameless: конфиг hypr-themes и файл текущей темы.
    const QString config = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/hypr-themes";
    const QString state =
        QStandardPaths::writableLocation(QStandardPaths::GenericStateLocation) + "/hypr-theme/current";
    QFile f(state);
    QString id;
    if (f.open(QIODevice::ReadOnly))
        id = QString::fromUtf8(f.readAll()).trimmed();
    // Защита от выхода за пределы каталога тем через идентификатор.
    if (id.contains('/') || id.contains(".."))
        id.clear();
    QString themeFile = config + "/themes/" + id + ".json";
    const auto theme = readJson(themeFile);
    QString palette = theme.value("palette").toString();
    if (palette.contains('/') || palette.contains(".."))
        palette.clear();
    const QString paletteFile = config + "/palettes/" + palette + ".json";
    const auto colors = readJson(paletteFile).value("colors").toObject();
    // Цвета берём из темы, при отсутствии — из палитры, иначе — запасные значения по умолчанию.
    QColor a(theme.value("accent").toString(colors.value("blue").toString("#b9a1ff")));
    QColor b(theme.value("accent2").toString(colors.value("lavender").toString("#74d9ce")));
    m_accent = a.isValid() ? a : QColor("#b9a1ff");
    m_secondary = b.isValid() ? b : QColor("#74d9ce");
    m_connected = !theme.isEmpty();
    m_name = theme.value("name").toString("Aurora");
    // Пересобираем список отслеживаемых путей, чтобы следить и за каталогами (создание/удаление тем).
    if (!m_watcher.files().isEmpty())
        m_watcher.removePaths(m_watcher.files());
    if (!m_watcher.directories().isEmpty())
        m_watcher.removePaths(m_watcher.directories());
    for (const auto& path :
         QStringList{state, themeFile, paletteFile, config, config + "/themes", QFileInfo(state).absolutePath()})
        if (QFileInfo::exists(path))
            m_watcher.addPath(path);
    emit themeChanged();
}
