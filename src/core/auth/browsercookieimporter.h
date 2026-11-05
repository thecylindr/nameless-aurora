#pragma once
// Импорт cookies из профилей браузеров: перечисление профилей и чтение сессии
// через Python-скрипт, который расшифровывает защищённое хранилище.
#include <QJsonArray>
#include <QObject>
#include <QProcess>
#include <QVariantList>

class SettingsStore;

class BrowserCookieImporter : public QObject
{
    Q_OBJECT
  public:
    // profileHome задаёт корень домашней директории (для тестов); по умолчанию — системный.
    explicit BrowserCookieImporter(SettingsStore* settings, QObject* parent = nullptr, const QString& profileHome = {});
    ~BrowserCookieImporter() override;
    // Собирает доступные профили браузеров с их базами cookies.
    static QVariantList profiles(const QString& home, const QString& configRoot = {});
    // Преобразует код ошибки импорта в понятный пользователю текст.
    static QString errorText(const QString& code);
    // Возвращает профили, доступные для текущего пользователя.
    QVariantList availableProfiles() const;
    // Запускает чтение сессии сервиса из указанного профиля (или подбирает его).
    void read(const QString& service, const QString& profileId = {});
    // Прерывает текущее чтение.
    void cancel();
  signals:
    // Текущий статус чтения (для показа в интерфейсе).
    void statusChanged(const QString& status);
    // Чтение завершено: cookies либо текст ошибки.
    void finished(const QString& service, const QString& profileId, const QJsonArray& cookies, const QString& error);

  private:
    // Переходит к следующему кандидату в списке профилей.
    void next();
    // Завершает чтение и отдаёт результат (или ошибку).
    void complete(const QJsonArray& cookies, const QString& error = {});
    // Находит интерпретатор Python 3 (предпочтительно управляемый venv).
    QString python() const;
    SettingsStore* m_settings;
    QProcess* m_process = nullptr;
    QVariantList m_candidates;
    QVariantMap m_current;
    QString m_service, m_error;
    QString m_profileHome;
    QByteArray m_output;
    bool m_explicit = false;
};
