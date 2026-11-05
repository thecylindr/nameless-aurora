#pragma once
#include <QWebEngineUrlRequestInterceptor>
// Перехватчик сетевых запросов WebEngine: ловит запросы к аудиофайлам, чтобы затем перехватывать поток воспроизведения.
class MediaSniffer : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT
  public:
    explicit MediaSniffer(const QString& service, QObject* parent = nullptr)
        : QWebEngineUrlRequestInterceptor(parent), m_service(service)
    {}
    void interceptRequest(QWebEngineUrlRequestInfo& info) override;
    // Возвращает true, если URL похож на прямой аудиофайл (по расширению или служебному пути).
    static bool isAudioUrl(const QUrl& url);

  private:
    const QString m_service;
  signals:
    void audioRequested(const QUrl& url);
};
