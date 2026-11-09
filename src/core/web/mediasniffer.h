#pragma once
#include <QObject>
#include <QUrl>
#ifdef AURORA_WEBENGINE
#include <QWebEngineUrlRequestInterceptor>
#endif
// Перехватчик сетевых запросов WebEngine: ловит запросы к аудиофайлам, чтобы затем перехватывать поток воспроизведения.
#ifdef AURORA_WEBENGINE
class MediaSniffer : public QWebEngineUrlRequestInterceptor
#else
class MediaSniffer : public QObject
#endif
{
    Q_OBJECT
  public:
    explicit MediaSniffer(const QString& service, QObject* parent = nullptr)
#ifdef AURORA_WEBENGINE
        : QWebEngineUrlRequestInterceptor(parent), m_service(service)
#else
        : QObject(parent), m_service(service)
#endif
    {}
#ifdef AURORA_WEBENGINE
    void interceptRequest(QWebEngineUrlRequestInfo& info) override;
#endif
    // Возвращает true, если URL похож на прямой аудиофайл (по расширению или служебному пути).
    static bool isAudioUrl(const QUrl& url);

  private:
    const QString m_service;
  signals:
    void audioRequested(const QUrl& url);
};
