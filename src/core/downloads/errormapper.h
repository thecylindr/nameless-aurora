#pragma once

#include <QRegularExpression>
#include <QString>

// Преобразует «сырой» вывод загрузчиков в понятные пользователю сообщения об ошибках.
namespace DownloaderErrors
{

inline QString humanizeDownloaderError(const QString& output, const QString& browser = QString())
{
    const QString text = output.toLower();

    if ((text.contains(QStringLiteral("could not find")) && text.contains(QStringLiteral("cookies database"))) ||
        text.contains(QStringLiteral("failed to load cookies"))) {
        if (browser.isEmpty()) {
            return QStringLiteral("Не удалось прочитать cookies браузера. Войдите на сайт в этом браузере "
                                  "и закройте все его окна, либо выберите другой браузер в настройках.");
        }
        return QStringLiteral("Не удалось прочитать cookies браузера «%1». Войдите на сайт в этом браузере "
                              "и закройте все его окна, либо выберите другой браузер в настройках.")
            .arg(browser);
    }
    if (text.contains(QStringLiteral("sign in to confirm")) || text.contains(QStringLiteral("not a bot"))) {
        return QStringLiteral("YouTube запросил проверку «не робот». Подождите минуту и повторите, "
                              "либо включите авторизованный режим с разрешённым браузером.");
    }
    if (text.contains(QStringLiteral("video unavailable")) || text.contains(QStringLiteral("private video"))) {
        return QStringLiteral("Видео недоступно (удалено или приватное).");
    }
    if (text.contains(QStringLiteral("http error 403"))) {
        return QStringLiteral(
            "Сервис отказал в доступе (403). Для личных плейлистов используйте авторизованный режим.");
    }
    if (text.contains(QStringLiteral("unable to extract")) || text.contains(QStringLiteral("unsupported url"))) {
        return QStringLiteral("Ссылка не распознана загрузчиком. Проверьте адрес.");
    }

    // Если конкретная причина не распознана, показываем последнюю строку вывода (обрезанную).
    const QStringList lines = output.split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
    QString tail = lines.isEmpty() ? QString() : lines.constLast().trimmed();
    if (tail.size() > 300) {
        tail = tail.left(297) + QStringLiteral("…");
    }
    return tail;
}

} // namespace DownloaderErrors
