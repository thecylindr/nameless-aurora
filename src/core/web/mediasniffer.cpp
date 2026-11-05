#include "mediasniffer.h"
#include "../auth/webloginmanager.h"
#include <QRegularExpression>
#include <QUrl>
bool MediaSniffer::isAudioUrl(const QUrl& url)
{
    // Только безопасные https-ссылки без встроенных учётных данных.
    if (url.scheme() != "https" || url.host().isEmpty() || !url.userInfo().isEmpty())
        return false;
    // Считаем аудио всё с типовым расширением, а также служебные пути get-mp3/get-mp4 (VK и др.).
    static const QRegularExpression extension("\\.(mp3|m4a|mp4|webm|aac|ogg|opus|flac|wav|m3u8)$",
                                              QRegularExpression::CaseInsensitiveOption);
    return extension.match(url.path()).hasMatch() ||
           (url.path().contains("/get-mp3/") || url.path().contains("/get-mp4/"));
}
void MediaSniffer::interceptRequest(QWebEngineUrlRequestInfo& info)
{
    // Игнорируем запросы с чужих доменов: нас интересует только трафик текущего сервиса.
    if (WebLoginManager::serviceForHost(info.firstPartyUrl().host()) != m_service)
        return;
    if (isAudioUrl(info.requestUrl()))
        emit audioRequested(info.requestUrl());
}
