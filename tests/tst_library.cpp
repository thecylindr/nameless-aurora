#include <QtTest>
#include <utility>
#include <QApplication>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QUrlQuery>
#include "web/mediasniffer.h"
#include "web/servicepayload.h"
#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QStandardPaths>
#include "librarymodel.h"
#include "homefeed.h"
#include "libraryexchange.h"
#include "importmanager.h"
#include "playbackcontroller.h"
#include "audioreactivity.h"
#include <cmath>
#include "settingsstore.h"
#include "storage/storagemanager.h"
#include "integration/namelessbridge.h"
#include "downloads/dependencymanager.h"
#include "downloads/errormapper.h"
#include "downloads/importpreviewer.h"
#include "downloads/audiosources.h"
#include "downloads/opencatalog.h"
#include "downloadmanager.h"
#include "auth/browserauthmanager.h"
#include "auth/oauthmanager.h"
#include "auth/cdpclient.h"
#include "auth/browsersession.h"
#include "auth/browsercookieimporter.h"
#include <QScopeGuard>
#include "auth/webloginmanager.h"
#include "vkprovider.h"


#ifndef Q_MOC_RUN
// A deterministic HTTP transport exercises the real download pipeline without sockets.
class AudioReply : public QNetworkReply {
public:
    AudioReply(const QNetworkRequest &request,QByteArray payload,int status,int delay,QObject *parent)
        :QNetworkReply(parent),m_payload(std::move(payload)) {
        setRequest(request);setUrl(request.url());setOperation(QNetworkAccessManager::GetOperation);
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute,status);setHeader(QNetworkRequest::ContentLengthHeader,m_payload.size());
        open(QIODevice::ReadOnly);
        QTimer::singleShot(delay,this,[this,status] {
            if(status>=400)setError(QNetworkReply::ContentAccessDenied,"Fixture access denied");
            emit readyRead();emit downloadProgress(m_payload.size(),m_payload.size());setFinished(true);emit finished();
        });
    }
    void abort() override {setError(OperationCanceledError,"Cancelled");setFinished(true);emit finished();}
    qint64 bytesAvailable() const override {return m_payload.size()-m_offset+QIODevice::bytesAvailable();}
protected:
    qint64 readData(char *data,qint64 size) override {
        const auto count=qMin(size,qint64(m_payload.size()-m_offset));if(count<=0)return -1;
        memcpy(data,m_payload.constData()+m_offset,size_t(count));m_offset+=count;return count;
    }
private:
    QByteArray m_payload;qint64 m_offset=0;
};
class AudioTransport : public QNetworkAccessManager {
public:
    QByteArray payload;int status=200,delay=0,requests=0;QNetworkRequest lastRequest;
    QList<int> responseStatuses;
protected:
    QNetworkReply *createRequest(Operation,const QNetworkRequest &request,QIODevice *) override {
        ++requests;lastRequest=request;return new AudioReply(request,payload,responseStatuses.isEmpty()?status:responseStatuses.takeFirst(),delay,this);
    }
};

class CatalogTransport : public QNetworkAccessManager {
public:
    QList<QUrl> requested;
protected:
    QNetworkReply *createRequest(Operation,const QNetworkRequest &request,QIODevice *) override {
        requested.append(request.url());QJsonObject response;
        if(request.url().path()=="/advancedsearch.php")response={{"response",QJsonObject{{"docs",QJsonArray{QJsonObject{{"identifier","public-song"}},QJsonObject{{"identifier","restricted-song"}}}}}}};
        else response={{"metadata",QJsonObject{{"identifier","public-song"},{"creator","Artist"}}},{"files",QJsonArray{QJsonObject{{"name","song.mp3"},{"title","Song"},{"length","200.0"}}}}};
        if(request.url().path().endsWith("restricted-song"))response["is_access_restricted"]=true;
        return new AudioReply(request,QJsonDocument(response).toJson(),200,0,this);
    }
};


class BrowserProtocol : public CdpClient {
public:
    QJsonObject harvestOverride;
    QString rejectedCookie;
    bool stallBrowserVersion=false;
    QJsonObject responseBody;
    std::function<QJsonObject(const QJsonObject &)> apiFixture;
    std::function<QJsonObject(const QJsonObject &)> publicFixture;
    QString publicSearchNavigation;
    int navigationFailures=0;
    QString navigationError="Not attached to an active page";
    bool fallbackWithoutEvents=false;
    int fallbackContextFailures=0;
    bool fallbackException=false;
    QString navigationUrl;
    QStringList navigationHistory;
    bool active=false;int next=0;QList<QPair<QString,QJsonObject>> commands;
    void connectToHost(const QUrl &,int) override {QTimer::singleShot(0,this,[this]{active=true;emit connected();});}
    bool isConnected() const override {return active;}
    void close() override {active=false;}
    int sendCommand(const QString &method,const QJsonObject &params={}) override {
        const int id=++next;commands.append({method,params});
        QTimer::singleShot(0,this,[this,id,method,params] {
            if(!active)return;QJsonObject result;
            if(method=="Page.navigate") {
                if(navigationFailures-->0){emit commandError(id,navigationError,-32000);return;}
                navigationUrl=params.value("url").toString();navigationHistory.append(navigationUrl);result={{"frameId","main"}};
            }
            else if(method=="Browser.getVersion") {
                if(stallBrowserVersion)return;
                result={{"userAgent","Test Chromium"}};
            }
            else if(method=="Network.setCookie") {
                if(params.value("name").toString()==rejectedCookie){emit commandError(id,"Invalid cookie fields: private-token");return;}
                result={{"success",true}};
            }
            else if(method=="Network.getResponseBody")result={{"body",QString::fromUtf8(QJsonDocument(responseBody).toJson(QJsonDocument::Compact))}};
            else if(method=="Runtime.evaluate") {
                const QString expression=params.value("expression").toString();QString value;
                if(expression=="navigator.userAgent"){emit commandError(id,"Command timed out");return;}
                if(expression=="navigation-fixture"){emit commandError(id,"Execution context was destroyed.");return;}
                if(expression.contains("const auroraNavigateTo=")) {
                    if(fallbackContextFailures-->0){emit commandError(id,"Cannot find default execution context",-32000);return;}
                    if(fallbackException){emit commandResult(id,{{"exceptionDetails",QJsonObject{{"text","private-error"}}}});return;}
                    const auto start=expression.indexOf("const auroraNavigateTo=")+QString("const auroraNavigateTo=").size();
                    navigationUrl=QJsonDocument::fromJson(expression.mid(start,expression.indexOf("[0];",start)-start).toUtf8()).array().first().toString();
                    navigationHistory.append(navigationUrl);
                    emit commandResult(id,{{"result",QJsonObject{{"value",true}}}});
                    if(!fallbackWithoutEvents) {
                        emit eventReceived("Page.frameNavigated",{{"frame",QJsonObject{{"id","main"},{"url",navigationUrl}}}});
                        emit eventReceived("Page.domContentEventFired",{});
                    }return;
                }
                if(expression.contains("auroraNavigationProbe"))value=QString::fromUtf8(QJsonDocument(QJsonObject{{"url",navigationUrl},{"ready","complete"}}).toJson(QJsonDocument::Compact));
                else if(expression=="navigator.userAgent")value="Test Chromium";
                else if(expression.contains("const options = ") && publicFixture) {
                    const int start=expression.indexOf("const options = ")+QString("const options = ").size();
                    const auto args=QJsonDocument::fromJson(expression.mid(start,expression.indexOf(";\n",start)-start).toUtf8()).object();
                    if(!args.value("submitted").toBool() && !args.value("resolve").toBool() && !publicSearchNavigation.isEmpty()) {
                        navigationUrl = std::exchange(publicSearchNavigation, QString());
                        emit commandError(id,"Execution context was destroyed");
                        emit eventReceived("Page.frameNavigated",{{"frame",QJsonObject{{"id","main"},{"url",navigationUrl}}}});
                        emit eventReceived("Page.domContentEventFired",{});return;
                    }
                    value=QString::fromUtf8(QJsonDocument(publicFixture(args)).toJson(QJsonDocument::Compact));
                }
                else if(expression.startsWith("(async()=>{const a=") && apiFixture) {
                    const int start=QString("(async()=>{const a=").size();
                    const auto args=QJsonDocument::fromJson(expression.mid(start,expression.indexOf(";const c=",start)-start).toUtf8()).object();
                    value=QString::fromUtf8(QJsonDocument(apiFixture(args)).toJson(QJsonDocument::Compact));
                }
                else if(expression.contains("authenticated:"))value=QString::fromUtf8(QJsonDocument(QJsonObject{{"authenticated",true}}).toJson(QJsonDocument::Compact));
                else if(expression.contains("const requested"))value=QString::fromUtf8(QJsonDocument(QJsonObject{{"activated",true},{"mediaUrl","https://cdn.example/song.wav"}}).toJson(QJsonDocument::Compact));
                else if(!harvestOverride.isEmpty())value=QString::fromUtf8(QJsonDocument(harvestOverride).toJson(QJsonDocument::Compact));
                else value=QString::fromUtf8(QJsonDocument(QJsonObject{{"atBottom",true},{"complete",true},{"loading",false},{"title","Native collection"},{"tracks",QJsonArray{QJsonObject{{"title","Native"},{"durationMs",12000},{"url","https://vk.ru/audio99_10"}}}}}).toJson(QJsonDocument::Compact));
                result={{"result",QJsonObject{{"type","string"},{"value",value}}}};
            } else if(method=="Storage.getCookies")result={{"cookies",QJsonArray{QJsonObject{{"name","remixsid6"},{"value","fixture"},{"domain",".vk.ru"},{"path","/"},{"secure",true}}}}};
            else if(method=="Browser.getWindowForTarget")result={{"windowId",7}};
            emit commandResult(id,result);
            if(method=="Page.navigate") {
                emit eventReceived("Page.frameNavigated",{{"frame",QJsonObject{{"id","main"},{"url",params.value("url")}}}});
                emit eventReceived("Page.domContentEventFired",{});
            }
        });return id;
    }
};
class DevToolsTransport : public QNetworkAccessManager {
protected:
    QNetworkReply *createRequest(Operation,const QNetworkRequest &request,QIODevice *) override {
        return new AudioReply(request,"[{\"id\":\"owned-page\",\"type\":\"page\",\"url\":\"about:blank\",\"webSocketDebuggerUrl\":\"ws://127.0.0.1:9222/devtools/page/owned-page\"}]",200,0,this);
    }
};
#endif

class LibraryTests : public QObject {
    Q_OBJECT
    static QString wav(const QString &path, qint16 sample=1) {
        QFile file(path);if(!file.open(QIODevice::WriteOnly))return {};
        QDataStream s(&file);s.setByteOrder(QDataStream::LittleEndian);
        const int bytes=44100*2*12;
        s.writeRawData("RIFF",4);s<<quint32(36+bytes);s.writeRawData("WAVEfmt ",8);s<<quint32(16)<<quint16(1)<<quint16(1)<<quint32(44100)<<quint32(88200)<<quint16(2)<<quint16(16);
        s.writeRawData("data",4);s<<quint32(bytes);for(int i=0;i<bytes/2;++i)s<<sample;
        return path;
    }
    static QString browserProfile(const QString &home) {
        const QString root=home+"/.mozilla/firefox";const QString profile=root+"/active";
        QDir().mkpath(profile);QSettings ini(root+"/profiles.ini",QSettings::IniFormat);
        ini.setValue("Profile0/Name","Already signed in");ini.setValue("Profile0/Path","active");
        ini.setValue("Profile0/IsRelative",1);ini.setValue("Profile0/Default",1);ini.sync();
        const QString connection="fixture-"+home;
        bool ok=false;{
            auto db=QSqlDatabase::addDatabase("QSQLITE",connection);db.setDatabaseName(profile+"/cookies.sqlite");
            if(db.open()){QSqlQuery query(db);
                ok=query.exec("CREATE TABLE moz_cookies(host TEXT,name TEXT,value TEXT,path TEXT,expiry INTEGER,isSecure INTEGER,isHttpOnly INTEGER)") &&
                   query.exec("INSERT INTO moz_cookies VALUES('.vk.ru','remixsid6','fixture-session','/',0,1,1)") &&
                   query.exec("INSERT INTO moz_cookies VALUES('.example.com','unrelated','private','/',0,1,1)");
            }
        }QSqlDatabase::removeDatabase(connection);return ok?profile:QString();
    }
private slots:
    void homePreferencesFilterRenameAndPersist()
    {
        QSettings stored;
        const auto previous = stored.value("home/recommendations");
        const auto restore = qScopeGuard([&] { stored.setValue("home/recommendations", previous); });
        stored.remove("home/recommendations");
        QTemporaryDir dir;
        LibraryModel library(nullptr, dir.filePath("library.db"));
        const int imported = library.createPlaylist("Импортированный плейлист");
        const int liked = library.createPlaylist("Мне нравится");
        Track track;
        track.title = "Local";
        track.artist = "Garden";
        track.filePath = wav(dir.filePath("local.wav"));
        const int local = library.repository().saveTrack(track);
        library.repository().addToPlaylist(imported, local);
        track.id = 0;
        track.title = "Remote";
        track.filePath.clear();
        track.sourceUrl = "https://vk.ru/audio99_2";
        const int remote = library.repository().saveTrack(track);
        library.repository().addToPlaylist(liked, remote);
        library.refresh();
        const auto mixes = HomeFeed::buildSections(&library, QDate(2026, 11, 6))[0].toMap()["mixes"].toList();
        for (const auto& entry : mixes) {
            const auto mix = entry.toMap();
            if (mix["playlistId"].toInt() == imported) {
                QCOMPARE(mix["title"].toString(), QString("Микс · Garden"));
                QCOMPARE(mix["playlistName"].toString(), QString("Импортированный плейлист"));
            }
            if (mix["playlistId"].toInt() == liked)
                QCOMPARE(mix["title"].toString(), QString("Любимое сегодня"));
        }
        const auto offline = HomeFeed::buildSections(&library, QDate(2026, 11, 6), {{"onlyDownloaded", true}});
        QCOMPARE(offline[0].toMap()["mixes"].toList()[0].toMap()["trackIds"].toList(), QVariantList{local});
        const auto excluded = HomeFeed::buildSections(&library, QDate(2026, 11, 6),
            {{"excludedPlaylists", QVariantList{imported, liked}}, {"includeUnsorted", false}});
        QVERIFY(excluded.isEmpty());
        SettingsStore settings;
        OAuthManager accounts(&settings);
        WebLoginManager web(&settings);
        PlaybackController player(&library, &settings);
        HomeFeed feed(&library, &accounts, &web, &player);
        feed.setPreferences({{"waveSource", "vk"}, {"discoveryMode", "familiar"}, {"showYandex", false}});
        HomeFeed reopened(&library, &accounts, &web, &player);
        QCOMPARE(reopened.preferences()["waveSource"].toString(), QString("vk"));
        QCOMPARE(reopened.preferences()["discoveryMode"].toString(), QString("familiar"));
        QVERIFY(!reopened.preferences()["showYandex"].toBool());
        feed.setPreferences({{"waveSource", "invalid"}});
        QCOMPARE(feed.preferences()["waveSource"].toString(), QString("auto"));
    }

    void mixedQueueKeepsLocalAndOnlineNavigation()
    {
        QTemporaryDir dir;
        SettingsStore settings;
        settings.setVolume(0);
        LibraryModel library(nullptr, dir.filePath("library.db"));
        Track track;
        track.title = "Local";
        track.filePath = wav(dir.filePath("local.wav"));
        const int local = library.repository().saveTrack(track);
        library.refresh();
        PlaybackController player(&library, &settings);
        QSignalSpy requested(&player, &PlaybackController::streamRequested);
        player.playStreamQueue({QVariantMap{{"libraryId", local}, {"title", "Local"}},
            QVariantMap{{"url", "https://vk.ru/audio99_2"}, {"title", "Online"}}});
        QCOMPARE(player.currentTrackId(), local);
        QCOMPARE(player.queue().size(), 2);
        library.setSearchText("hidden");
        player.next();
        QCOMPARE(requested.last()[0].toMap()["title"].toString(), QString("Online"));
        player.previous();
        QCOMPARE(player.currentTrackId(), local);
        QCOMPARE(player.queue().size(), 2);
        player.playQueueTrack(-1001);
        QCOMPARE(requested.last()[0].toMap()["title"].toString(), QString("Online"));
        player.previous();
        player.removeFromQueue(-1001);
        QCOMPARE(player.currentTrackId(), local);
        QCOMPARE(player.queue().size(), 1);
        player.stop();
    }

    void previewCachePreservesRequestMetadataAndCanBeInvalidated()
    {
        AudioSources preview(nullptr, nullptr, nullptr, nullptr, {}, true);
        QSignalSpy resolved(&preview, &AudioSources::previewResolved);
        QVariantMap first{{"url", "https://www.youtube.com/watch?v=cache-fixture"},
                          {"source", "YouTube"}, {"title", "First title"}, {"queueId", -1001},
                          {"mediaUrl", "https://cdn.example/song.m4a"}, {"mediaVerified", true},
                          {"streamHeaders", QVariantMap{{"User-Agent", "Aurora fixture"}}}};
        preview.previewCandidate(first);
        QCOMPARE(resolved.count(), 1);
        QVERIFY(resolved.last()[1].toString().isEmpty());
        auto second = first;
        second.remove("mediaUrl");
        second.remove("mediaVerified");
        second.remove("streamHeaders");
        second["queueId"] = -2002;
        second["title"] = "Updated title";
        preview.previewCandidate(second);
        QCOMPARE(resolved.count(), 2);
        const auto cached = resolved.last()[0].toMap();
        QCOMPARE(cached.value("title").toString(), QString("Updated title"));
        QCOMPARE(cached.value("queueId").toInt(), -2002);
        QCOMPARE(cached.value("mediaUrl"), first.value("mediaUrl"));
        QCOMPARE(cached.value("streamHeaders"), first.value("streamHeaders"));
        QVERIFY(!preview.busy());
        preview.invalidatePreview(first.value("url").toString());
        preview.previewCandidate(second);
        QCOMPARE(resolved.count(), 3);
        QVERIFY(!resolved.last()[1].toString().isEmpty());
        QVERIFY(resolved.last()[0].toMap().isEmpty());
    }

    void onlineFallbackTriesEnabledServicesWithoutDownloadingAndCancels()
    {
#ifdef Q_OS_WIN
        QSKIP("Fixture executable uses a POSIX launcher");
#else
        QTemporaryDir dir;
        const auto oldSources = QSettings().value("downloads/sourceOrder");
        const auto restore = qScopeGuard([&] {
            if (oldSources.isValid()) QSettings().setValue("downloads/sourceOrder", oldSources);
            else QSettings().remove("downloads/sourceOrder");
        });
        QSettings().setValue("downloads/sourceOrder", QStringList{"YouTube", "SoundCloud"});
        const QString log = dir.filePath("online-attempts.jsonl");
        const auto quotedLog = QString::fromUtf8(QJsonDocument(QJsonArray{log}).toJson(QJsonDocument::Compact));
        const auto executable = dir.filePath("yt-dlp");
        QFile tool(executable);
        QVERIFY(tool.open(QIODevice::WriteOnly));
        tool.write(("#!/usr/bin/env python3\nimport sys,json,pathlib\na=sys.argv[1:]\n"
            "if '--version' in a: print('2026.11.05');sys.exit(0)\n"
            "if a[-1].startswith(('ytsearch','scsearch')):\n"
            " url='https://www.youtube.com/watch?v=fixture' if a[-1].startswith('ytsearch') else 'https://soundcloud.com/fixture/song'\n"
            " print(json.dumps({'entries':[{'title':'Artist - A song','uploader':'Artist','duration':12,'url':url}]}));sys.exit(0)\n"
            "with open(" + quotedLog + "[0],'a') as f: f.write(json.dumps(a[-1])+'\\n')\n"
            "print('fixture stream unavailable',file=sys.stderr);sys.exit(1)\n").toUtf8());
        tool.close();
        QVERIFY(tool.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        SettingsStore settings;
        settings.setLibraryFolder(dir.filePath("music"));
        settings.setYtDlpPath(executable);
        LibraryModel library(nullptr, dir.filePath("fallback.db"));
        Track track;
        track.title = "A song"; track.artist = "Artist"; track.duration = 12000;
        track.source = "VK"; track.sourceUrl = "https://vk.ru/audio99_42";
        const int id = library.repository().saveTrack(track);
        library.refresh();
        OAuthManager accounts(&settings);
        PlaybackController player(&library, &settings);
        WebLoginManager web(&settings);
        web.logout("vk");
        DependencyManager tools(&settings);
        QVERIFY(tools.runtimeReady());
        DownloadManager downloads(nullptr, nullptr, nullptr, nullptr);
        AudioSources preview(&library, &downloads, &tools, nullptr, {}, true);
        HomeFeed feed(&library, &accounts, &web, &player, nullptr, nullptr, &preview);
        feed.playLibraryTrack(id);
        QTRY_VERIFY_WITH_TIMEOUT(QFileInfo::exists(log), 5000);
        QTRY_VERIFY_WITH_TIMEOUT(feed.resolvingId() == 0, 5000);
        QFile attempts(log); QVERIFY(attempts.open(QIODevice::ReadOnly));
        const auto before = attempts.readAll(); attempts.close();
        QVERIFY(before.contains("youtube.com"));
        QVERIFY(before.contains("soundcloud.com"));
        QCOMPARE(before.count('\n'), 2);
        QCOMPARE(downloads.rowCount(), 0);
        QCOMPARE(library.repository().count(), 1);
        QCOMPARE(library.trackById(id).sourceUrl, track.sourceUrl);
        QVERIFY(library.trackById(id).filePath.isEmpty());
        QCOMPARE(player.queue().size(), 1);
        feed.playLibraryTrack(id);
        auto* deadline = feed.findChild<QTimer*>("onlineResolutionDeadline");
        QVERIFY(deadline);
        QVERIFY(deadline->isActive());
        QVERIFY(QMetaObject::invokeMethod(deadline, "timeout", Qt::DirectConnection));
        QVERIFY(feed.playbackStatus().contains("45 секунд"));
        QCOMPARE(feed.resolvingId(), 0);
        QVERIFY(!deadline->isActive());
        player.stop();
        QTest::qWait(150);
        QVERIFY(feed.playbackStatus().isEmpty());
        QVERIFY(attempts.open(QIODevice::ReadOnly));
        QCOMPARE(attempts.readAll(), before);
#endif
    }

    void automaticOnlineMatchChecksVersionDurationAndIdentity()
    {
        const QVariantMap target{{"title", "Memory Reboot"}, {"artist", "VOJ, Narvent"}, {"durationMs", 209000}};
        QVariantMap candidate{{"title", "VØJ & Narvent - Memory Reboot (Official Music Video)"}, {"artist", "Narvent"}, {"durationMs", 211000}};
        QVERIFY(AudioSources::automaticMatch(target, candidate));
        candidate["title"] = "VOJ Narvent Memory Reboot Slowed Remix";
        QVERIFY(!AudioSources::automaticMatch(target, candidate));
        candidate["title"] = "VOJ Narvent Memory Reboot";
        candidate["durationMs"] = 30000;
        QVERIFY(!AudioSources::automaticMatch(target, candidate));
        candidate["durationMs"] = 0;
        QVERIFY(AudioSources::automaticMatch(target, candidate));
        QVERIFY(!AudioSources::automaticMatch({{"title", "Intro"}}, {{"title", "Intro"}}));
    }

    void libraryOnlineQueueKeepsTracksWithoutFilesOrSourceUrls()
    {
        QTemporaryDir dir;
        SettingsStore settings;
        OAuthManager accounts(&settings);
        LibraryModel library(nullptr, dir.filePath("online-queue.db"));
        Track track;
        track.title = "First";
        track.artist = "Fixture";
        const int first = library.repository().saveTrack(track);
        track.id = 0;
        track.title = "Second";
        const int second = library.repository().saveTrack(track);
        QVERIFY(first > 0 && second > 0);
        library.refresh();
        PlaybackController player(&library, &settings);
        WebLoginManager web(&settings);
        HomeFeed feed(&library, &accounts, &web, &player);
        QSignalSpy requested(&player, &PlaybackController::streamRequested);
        feed.playLibraryTrack(second);
        QCOMPARE(player.queue().size(), 2);
        QCOMPARE(requested.last().at(0).toMap().value("libraryTrackId").toInt(), second);
        QCOMPARE(requested.last().at(0).toMap().value("key").toString(), "aurora:track/" + QString::number(second));
        library.setSearchText("Second");
        feed.playLibraryTrack(second);
        QCOMPARE(player.queue().size(), 1);
        player.stop();
        QVERIFY(requested.last().at(0).toMap().isEmpty());
        QCOMPARE(feed.resolvingId(), 0);
        QCOMPARE(library.repository().tracks().size(), 2);
        QVERIFY(library.trackById(second).filePath.isEmpty());
    }

    void homeVkRecommendationsStaySeparateFromSearch()
    {
#ifdef Q_OS_WIN
        QSKIP("Fixture Chromium launcher uses a POSIX shell");
#else
        QTemporaryDir dir;
        QFile browser(dir.filePath("brave"));
        QVERIFY(browser.open(QIODevice::WriteOnly));
        browser.write("#!/bin/sh\nexec sleep 30\n");
        browser.close();
        browser.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
        const auto oldPath = qgetenv("PATH");
        const auto restorePath = qScopeGuard([&] { qputenv("PATH", oldPath); });
        qputenv("PATH", dir.path().toUtf8() + ':' + oldPath);
        SettingsStore settings;
        OAuthManager accounts(&settings);
        LibraryModel library(nullptr, dir.filePath("library.db"));
        PlaybackController player(&library, &settings);
        DevToolsTransport network;
        BrowserProtocol protocol;
        QJsonArray tracks;
        for (int i = 0; i < 24; ++i)
            tracks.append(QJsonObject{{"title", "Personal " + QString::number(i)},
                {"artist", "VK artist"}, {"url", "https://vk.ru/audio99_" + QString::number(i + 1)}});
        protocol.harvestOverride = {{"tracks", tracks}, {"complete", true}, {"atBottom", true}};
        WebLoginManager web(&settings, nullptr, [&](QObject* parent) {
            return new BrowserSession(parent, &network, &protocol, [] { return 9222; });
        });
        web.saveCookies("vk", {{"remixsid6", "fixture", ".vk.ru", "/", 0, true, true}});
        const auto logout = qScopeGuard([&] { web.logout("vk"); });
        HomeFeed feed(&library, &accounts, &web, &player);
        QSignalSpy search(&web, &WebLoginManager::searchReady);
        QSignalSpy recommendations(&web, &WebLoginManager::vkRecommendationsReady);
        QVERIFY(feed.vkConnected());
        feed.refreshVk();
        QTRY_VERIFY_WITH_TIMEOUT(!feed.vkBusy(), 9000);
        QCOMPARE(feed.vkTracks().size(), 24);
        QCOMPARE(recommendations.count(), 1);
        QCOMPARE(search.count(), 0);
        bool explored = false;
        for (const auto& command : protocol.commands)
            if (command.first == "Page.navigate" &&
                QUrl(command.second.value("url").toString()).query() == "section=explore")
                explored = true;
        QVERIFY(explored);
        feed.refreshVk();
        QVERIFY(!feed.vkBusy());
        QCOMPARE(recommendations.count(), 1);
        web.logout("vk");
        QVERIFY(feed.vkTracks().isEmpty());
        QVERIFY(!feed.vkConnected());
#endif
    }

    void homeMixesRotateAndIncludeUnheardPlaylistTracks()
    {
        QTemporaryDir dir;
        LibraryModel library(nullptr, dir.filePath("library.db"));
        const int morning = library.createPlaylist("Утро");
        const int evening = library.createPlaylist("Вечер");
        for (int i = 0; i < 48; ++i) {
            Track track;
            track.title = "Track " + QString::number(i);
            track.artist = "Artist " + QString::number(i % 12);
            track.sourceUrl = "https://music.yandex.ru/track/" + QString::number(100 + i);
            track.playCount = i < 24 ? 1000 : 0;
            const int id = library.repository().saveTrack(track);
            QVERIFY(id > 0);
            if (i < 24)
                QVERIFY(library.repository().recordPlay(id));
            QVERIFY(library.repository().addToPlaylist(i % 2 ? morning : evening, id));
        }
        library.refresh();
        const QDate date(2026, 11, 5);
        const auto sections = HomeFeed::buildSections(&library, date);
        QCOMPARE(sections.size(), 3);
        QCOMPARE(sections, HomeFeed::buildSections(&library, date));
        QCOMPARE(sections[0].toMap().value("mixes").toList().first().toMap().value("count").toInt(), 18);
        QCOMPARE(sections[1].toMap().value("mixes").toList().first().toMap().value("count").toInt(), 36);
        QCOMPARE(sections[2].toMap().value("mixes").toList().first().toMap().value("count").toInt(), 48);
        for (const auto& section : sections) {
            const auto mixes = section.toMap().value("mixes").toList();
            QCOMPARE(mixes.size(), 3);
            for (const auto& entry : mixes) {
                const auto mix = entry.toMap();
                const int playlist = mix.value("playlistId").toInt();
                QSet<int> allowed, seen;
                for (const auto& track : library.repository().tracks({}, playlist))
                    allowed.insert(track.id);
                int unheard = 0;
                for (const auto& id : mix.value("trackIds").toList()) {
                    QVERIFY(allowed.contains(id.toInt()));
                    QVERIFY(!seen.contains(id.toInt()));
                    seen.insert(id.toInt());
                    unheard += library.trackById(id.toInt()).playCount == 0;
                }
                QVERIFY(unheard >= 6);
                QVERIFY(mix.value("playableIds").toList().isEmpty());
                QVERIFY(mix.value("covers").toList().size() <= 4);
            }
        }
        library.setPlaylistFilter(morning);
        library.setSearchText("nothing matches");
        QCOMPARE(HomeFeed::buildSections(&library, date), sections);
        const auto tomorrow = HomeFeed::buildSections(&library, date.addDays(1));
        QVERIFY(tomorrow[0].toMap().value("mixes") != sections[0].toMap().value("mixes"));
        QCOMPARE(tomorrow[2].toMap().value("mixes"), sections[2].toMap().value("mixes"));
        QDate threeDayStart = date.addDays(-(date.toJulianDay() % 3));
        QCOMPARE(HomeFeed::buildSections(&library, threeDayStart)[1].toMap().value("mixes"),
                 HomeFeed::buildSections(&library, threeDayStart.addDays(2))[1].toMap().value("mixes"));
        QVERIFY(HomeFeed::buildSections(&library, threeDayStart)[1].toMap().value("mixes")
                != HomeFeed::buildSections(&library, threeDayStart.addDays(3))[1].toMap().value("mixes"));
    }

    void homeSelectionKeepsItsOwnPlayableQueue()
    {
        QTemporaryDir dir;
        SettingsStore settings;
        settings.setVolume(0);
        LibraryModel library(nullptr, dir.filePath("library.db"));
        QVariantList ids;
        for (int i = 0; i < 3; ++i) {
            Track track;
            track.title = QString::number(i);
            track.filePath = wav(dir.filePath(track.title + ".wav"));
            ids.append(library.repository().saveTrack(track));
        }
        library.refresh();
        PlaybackController player(&library, &settings);
        player.playSelection({ids[1], ids[1], -999, ids[2]}, ids[1].toInt());
        QCOMPARE(player.currentTrackId(), ids[1].toInt());
        QCOMPARE(player.queue().size(), 2);
        library.setSearchText("hidden");
        player.next();
        QCOMPARE(player.currentTrackId(), ids[2].toInt());
        QCOMPARE(player.queue().size(), 2);
        player.previous();
        QCOMPARE(player.currentTrackId(), ids[1].toInt());
        QVERIFY(!player.shuffle());
        QSignalSpy requested(&player, &PlaybackController::streamRequested);
        player.playStreamQueue({QVariantMap{{"url", "https://music.yandex.ru/track/42"}}});
        player.playTrackId(ids[1].toInt());
        QVERIFY(requested.last()[0].toMap().isEmpty());
        QCOMPARE(player.currentTrackId(), ids[1].toInt());
        QVERIFY(!player.queue().isEmpty());
        player.stop();
    }

    void streamQueueHandlesNextPreviousRemovalAndCancellation()
    {
        QTemporaryDir dir;
        SettingsStore settings;
        LibraryModel library(nullptr, dir.filePath("library.db"));
        PlaybackController player(&library, &settings);
        QSignalSpy requested(&player, &PlaybackController::streamRequested);
        QSignalSpy exhausted(&player, &PlaybackController::streamQueueExhausted);
        const QVariantList tracks{QVariantMap{{"url", "https://music.yandex.ru/track/1"}, {"title", "One"}},
                                  QVariantMap{{"url", "https://music.yandex.ru/track/2"}, {"title", "Two"}},
                                  QVariantMap{{"url", "https://music.yandex.ru/track/3"}, {"title", "Three"}}};
        player.playStreamQueue(tracks);
        QCOMPARE(player.queue().size(), 3);
        QCOMPARE(requested.last()[0].toMap().value("title").toString(), QString("One"));
        player.next();
        QCOMPARE(requested.last()[0].toMap().value("title").toString(), QString("Two"));
        player.removeFromQueue(-1000);
        QCOMPARE(player.queue().size(), 2);
        player.next();
        QCOMPARE(requested.last()[0].toMap().value("title").toString(), QString("Three"));
        player.next();
        QCOMPARE(exhausted.count(), 1);
        player.previous();
        QCOMPARE(requested.last()[0].toMap().value("title").toString(), QString("Two"));
        player.playQueueTrack(-1002);
        QCOMPARE(requested.last()[0].toMap().value("title").toString(), QString("Three"));
        player.stop();
        QVERIFY(requested.last()[0].toMap().isEmpty());
        QVERIFY(player.queue().isEmpty());
    }

    void yandexWaveParsesOnlyAvailableUniqueTracks()
    {
        const auto track = [](int id) {
            return QJsonObject{{"id", id}, {"title", "Wave song"}, {"durationMs", 180000},
                               {"artists", QJsonArray{QJsonObject{{"name", "Wave artist"}}}},
                               {"coverUri", "avatars.yandex.net/get-music-content/test/%%"},
                               {"secret", "private-account-field"}};
        };
        auto unavailable = track(3);
        unavailable["available"] = false;
        const auto tracks = ServicePayload::yandexWave(QJsonObject{{"result", QJsonObject{{"sequence", QJsonArray{
            QJsonObject{{"track", track(1)}}, QJsonObject{{"track", track(1)}},
            QJsonObject{{"track", unavailable}}, QJsonObject{{"track", track(2)}}}}}}});
        QCOMPARE(tracks.size(), 2);
        QCOMPARE(tracks[0].sourceUrl, QString("https://music.yandex.ru/track/1"));
        QCOMPARE(tracks[1].artist, QString("Wave artist"));
        QVERIFY(tracks[0].coverUrl.endsWith("/400x400"));
        QVERIFY(!tracks[0].webData.contains("secret"));
        QVERIFY(ServicePayload::yandexWave(QJsonObject{{"error", "denied"}}).isEmpty());
        QVERIFY(BrowserSession::isWaveApi(QUrl("https://api.music.yandex.ru/rotor/station/user:onyourwave/tracks")));
        QVERIFY(!BrowserSession::isWaveApi(QUrl("https://api.music.yandex.ru.evil.test/rotor/station/user:onyourwave/tracks")));
        QVERIFY(!BrowserSession::isWaveApi(QUrl("http://api.music.yandex.ru/rotor/station/user:onyourwave/tracks")));
        QVERIFY(!BrowserSession::isWaveApi(QUrl("https://api.music.yandex.ru/account")));
    }

    void homeYandexUsesAccountAndCacheAndDiscardsLateResponses()
    {
        QTemporaryDir dir;
        SettingsStore settings;
        settings.setYandexToken({});
        LibraryModel library(nullptr, dir.filePath("library.db"));
        OAuthManager accounts(&settings);
        WebLoginManager web(&settings);
        web.logout("yandex");
        PlaybackController player(&library, &settings);
        AudioTransport network;
        network.payload = R"({"result":{"sequence":[{"track":{"id":"42","title":"From Yandex","artists":[{"name":"Artist"}]}}]}})";
        HomeFeed feed(&library, &accounts, &web, &player, nullptr, &network);
        feed.refreshYandex();
        QCOMPARE(network.requests, 0);
        QVERIFY(feed.yandexTracks().isEmpty());
        accounts.saveToken("yandex", "fixture-token");
        feed.playWave();
        QVERIFY(feed.busy());
        player.stop();
        QTRY_VERIFY(!feed.busy());
        QVERIFY(player.queue().isEmpty());
        QVERIFY(!web.busy());
        QCOMPARE(feed.yandexTracks().size(), 1);
        QCOMPARE(feed.yandexTracks().first().toMap().value("title").toString(), QString("From Yandex"));
        QCOMPARE(network.lastRequest.rawHeader("Authorization"), QByteArray("OAuth fixture-token"));
        QCOMPARE(network.lastRequest.url().host(), QString("api.music.yandex.net"));
        feed.refreshYandex();
        QCOMPARE(network.requests, 1);
        network.status = 403;
        feed.refreshYandex(true);
        QTRY_VERIFY(!feed.busy());
        QCOMPARE(network.requests, 2);
        QCOMPARE(QUrlQuery(network.lastRequest.url()).queryItemValue("queue"), QString("42"));
        QCOMPARE(feed.yandexTracks().size(), 1);
        QVERIFY(feed.status().contains("Войди"));
        QVERIFY(!feed.status().contains("fixture-token"));
        network.status = 200;
        network.delay = 50;
        feed.refreshYandex(true);
        accounts.logout("yandex");
        QTest::qWait(100);
        QVERIFY(feed.yandexTracks().isEmpty());
        QVERIFY(!feed.busy());
        QVERIFY(!feed.yandexConnected());
        accounts.saveToken("yandex", "expired-fixture");
        settings.setYandexTokenExpiry(QDateTime::currentSecsSinceEpoch() - 1);
        feed.refreshYandex();
        QVERIFY(!feed.busy());
        QVERIFY(feed.yandexTracks().isEmpty());
        QVERIFY(feed.status().contains("Войди"));
    }

    void yandexWaveReadsThroughBrowserSession_data()
    {
        QTest::addColumn<int>("apiStatus");
        QTest::newRow("account-wave") << 200;
        QTest::newRow("expired-session") << 403;
    }

    void yandexWaveReadsThroughBrowserSession()
    {
#ifndef Q_OS_WIN
        QFETCH(int, apiStatus);
        QTemporaryDir dir;
        const auto executable = dir.filePath("brave");
        QFile file(executable);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("#!/bin/sh\nexec sleep 30\n");
        file.close();
        QVERIFY(file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        const auto oldPath = qgetenv("PATH");
        const auto restore = qScopeGuard([&] { qputenv("PATH", oldPath); });
        qputenv("PATH", dir.path().toUtf8() + ":" + oldPath);
        SettingsStore settings;
        DevToolsTransport network;
        BrowserProtocol protocol;
        bool waveRequested = false;
        protocol.apiFixture = [&](const QJsonObject& args) {
            waveRequested = QUrl(args.value("url").toString()).path() == "/rotor/station/user:onyourwave/tracks";
            return QJsonObject{{"status", apiStatus}, {"payload", QJsonObject{{"result", QJsonObject{
                {"sequence", QJsonArray{QJsonObject{{"track", QJsonObject{{"id", "19"}, {"title", "Personal wave"}}}}}}}}}}};
        };
        WebLoginManager web(&settings, nullptr, [&](QObject* parent) {
            return new BrowserSession(parent, &network, &protocol, [] { return 9222; });
        });
        web.saveCookies("yandex", {{"Session_id", "fixture-session", ".yandex.ru", "/", 0, true, true}});
        QSignalSpy ready(&web, &WebLoginManager::waveReady);
        web.requestWave();
        QTRY_COMPARE_WITH_TIMEOUT(ready.count(), 1, 9000);
        QVERIFY(waveRequested);
        QVERIFY(!web.busy());
        const auto tracks = qvariant_cast<QVector<Track>>(ready[0][0]);
        if (apiStatus == 200) {
            QCOMPARE(tracks.size(), 1);
            QCOMPARE(tracks[0].title, QString("Personal wave"));
            QVERIFY(ready[0][1].toString().isEmpty());
        } else {
            QVERIFY(tracks.isEmpty());
            QVERIFY(ready[0][1].toString().contains("Войди"));
        }
        QVERIFY(!ready[0][1].toString().contains("fixture-session"));
        web.logout("yandex");
#endif
    }

    void archiveCatalogKeepsOnlyPublicIndividualAudioFiles() {
        QJsonObject root{{"metadata",QJsonObject{{"identifier","album-2026"},{"creator","Artist"}}},
            {"files",QJsonArray{QJsonObject{{"name","01 song.mp3"},{"title","Song"},{"length","3:20"}},
                QJsonObject{{"name","private.mp3"},{"private",true}},QJsonObject{{"name","../escape.mp3"}},QJsonObject{{"name","cover.jpg"}}}}};
        const auto files=OpenCatalog::parseFiles(root);QCOMPARE(files.size(),1);const auto t=files.first().toMap();
        QCOMPARE(t.value("durationMs").toLongLong(),200000);QCOMPARE(t.value("artist").toString(),QString("Artist"));
        QCOMPARE(t.value("mediaUrl").toString(),QString("https://archive.org/download/album-2026/01%20song.mp3"));
        root["is_access_restricted"]=true;QVERIFY(OpenCatalog::parseFiles(root).isEmpty());
        root.remove("is_access_restricted");root["files"]=QJsonArray{QJsonObject{{"name","song.mp3"},{"length","inf"}}};
        QCOMPARE(OpenCatalog::parseFiles(root).first().toMap().value("durationMs").toLongLong(),0);
    }
    void archiveCatalogSearchResolvesPublicFilesWithoutBrowser() {
        CatalogTransport transport;OpenCatalog catalog(nullptr,&transport);QSignalSpy ready(&catalog,&OpenCatalog::ready);
        QSignalSpy failed(&catalog,&OpenCatalog::failed);catalog.search("Song","Artist");
        QTRY_COMPARE_WITH_TIMEOUT(ready.count(),1,2000);QCOMPARE(failed.count(),0);QCOMPARE(transport.requested.size(),3);
        const auto files=ready.first().first().toList();QCOMPARE(files.size(),1);
        QCOMPARE(files.first().toMap().value("mediaUrl").toString(),QString("https://archive.org/download/public-song/song.mp3"));
        QVERIFY(files.first().toMap().value("mediaVerified").toBool());
        QVERIFY(QUrlQuery(transport.requested.first()).queryItemValue("q").contains("mediatype:audio"));
    }
    void playbackClockAndSeekWorkWithEqualizerEnabled() {
        QTemporaryDir dir;SettingsStore settings;settings.setLibraryFolder(dir.filePath("music"));
        LibraryModel library(nullptr,dir.filePath("db"));QVERIFY(library.addTrack(wav(dir.filePath("track.wav"))));
        PlaybackController player(&library,&settings);player.equalizer()->selectPreset("Тёплый бас");player.equalizer()->setEnabled(true);
        player.playIndex(0);QTRY_VERIFY_WITH_TIMEOUT(player.position()>100,4000);player.seek(3000);
        QTRY_VERIFY_WITH_TIMEOUT(player.position()>=3000,2500);player.pause();QVERIFY(!player.playing());
        player.stop();player.equalizer()->setEnabled(false);player.equalizer()->reset();QVERIFY(player.lastError().isEmpty());
    }
    void equalizerChangesRealPcmAndKeepsChannelsSeparate() {
        AudioEqualizer eq;eq.reset();eq.setAutoHeadroom(false);eq.setEnabled(true);eq.setBand(5,6);
        QAudioFormat f;f.setSampleRate(48000);f.setChannelCount(2);f.setSampleFormat(QAudioFormat::Float);
        QByteArray pcm(48000*2*sizeof(float),0);auto *samples=reinterpret_cast<float*>(pcm.data());
        for(int i=0;i<48000;++i)samples[2*i]=float(.05*std::sin(i*2*3.141592653589793*1000/48000));
        const QAudioBuffer input(pcm,f);const auto output=eq.process(input);const auto *out=output.constData<float>();
        double inPower=0,outPower=0;for(int i=12000;i<48000;++i){inPower+=samples[2*i]*samples[2*i];outPower+=out[2*i]*out[2*i];QVERIFY(std::abs(out[2*i+1])<1e-7);}
        const auto gain=10*std::log10(outPower/inPower);QVERIFY(gain>5.8 && gain<6.2);
        eq.setEnabled(false);const auto flat=eq.process(input);QCOMPARE(QByteArray(flat.constData<char>(),flat.byteCount()),pcm);
        eq.reset();eq.setAutoHeadroom(true);
    }
    void equalizerBoundsPresetsAndPersistence() {
        AudioEqualizer eq;eq.reset();eq.setBand(0,999);QCOMPARE(eq.gains()[0].toDouble(),12.);
        eq.setBand(-1,9);eq.setPreamp(-999);QCOMPARE(eq.preamp(),-18.);
        eq.selectPreset("Вокал");QCOMPARE(eq.preset(),QString("Вокал"));QVERIFY(eq.gains()[5].toDouble()>0);
        AudioEqualizer restored;QCOMPARE(restored.gains(),eq.gains());QCOMPARE(restored.preset(),eq.preset());
        eq.reset();eq.setEnabled(false);
    }
    void storageScanIsDeferredAndIgnoresStaleFolderResults() {
        QTemporaryDir dir;const auto first=dir.filePath("first"),second=dir.filePath("second");
        QVERIFY(QDir().mkpath(first+"/nested"));QVERIFY(QDir().mkpath(second));
        QFile a(first+"/nested/audio.bin");QVERIFY(a.open(QIODevice::WriteOnly));a.write(QByteArray(12345,'a'));a.close();
        QFile b(second+"/audio.bin");QVERIFY(b.open(QIODevice::WriteOnly));b.write(QByteArray(321,'b'));b.close();
        SettingsStore settings;settings.setLibraryFolder(first);StorageManager storage(&settings);QVERIFY(storage.busy());
        settings.setLibraryFolder(second);QTRY_VERIFY_WITH_TIMEOUT(!storage.busy(),4000);
        QCOMPARE(storage.libraryFolder(),second);QCOMPARE(storage.libraryBytes(),321);QVERIFY(storage.totalBytes()>0);
        settings.setLibraryFolder(first);QTRY_VERIFY_WITH_TIMEOUT(!storage.busy(),4000);QCOMPARE(storage.libraryBytes(),12345);
    }
    void startupChecksDoNotBlockWindowOrLoseQueuedPreparation() {
#ifndef Q_OS_WIN
        QTemporaryDir dir;
        const auto oldPath=qgetenv("PATH");const auto restore=qScopeGuard([&]{qputenv("PATH",oldPath);});
        for(const auto &name:QStringList{"yt-dlp","spotdl","ffmpeg","ffprobe","node"}) {
            QFile file(dir.filePath(name));QVERIFY(file.open(QIODevice::WriteOnly));
            file.write(name=="yt-dlp"?"#!/bin/sh\n/bin/sleep 1\nprintf '2026.11.05\\n'\n":name=="node"?"#!/bin/sh\nprintf 'v24.0.0\\n'\n":"#!/bin/sh\nprintf 'fixture 1.0\\n'\n");
            file.close();QVERIFY(file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
        }
        qputenv("PATH",dir.path().toUtf8());SettingsStore settings;
        settings.setYtDlpPath(dir.filePath("yt-dlp"));settings.setSpotdlPath(dir.filePath("spotdl"));
        QElapsedTimer elapsed;elapsed.start();DependencyManager tools(&settings,nullptr,true);
        QVERIFY2(elapsed.elapsed()<250,"Version checks blocked construction of the window");QVERIFY(tools.busy());
        QSignalSpy prepared(&tools,&DependencyManager::preparationFinished);tools.ensureReady();
        bool responsive=false;QTimer::singleShot(20,&tools,[&]{responsive=true;});
        QTRY_VERIFY_WITH_TIMEOUT(responsive,500);QVERIFY(tools.busy());
        QTRY_COMPARE_WITH_TIMEOUT(prepared.count(),1,8000);QVERIFY(prepared.first().first().toBool());QVERIFY(tools.runtimeReady());
#endif
    }
    void automaticSourcesTryNextServiceAndAttachPlayableAudio() {
#ifndef Q_OS_WIN
        QTemporaryDir dir;const auto input=wav(dir.filePath("fixture.wav"));const auto executable=dir.filePath("yt-dlp");
        QFile file(executable);QVERIFY(file.open(QIODevice::WriteOnly));
        const auto quoted=QString::fromUtf8(QJsonDocument(QJsonArray{input}).toJson(QJsonDocument::Compact));
        file.write(("#!/usr/bin/env python3\nimport sys,json,pathlib,shutil\na=sys.argv[1:]\n"
            "if '--version' in a: print('2026.11.05');sys.exit(0)\n"
            "if '--dump-single-json' in a:\n"
            " url='https://www.youtube.com/watch?v=abc123' if a[-1].startswith('ytsearch') else 'https://soundcloud.com/artist/song'\n"
            " print(json.dumps({'entries':[{'title':'Artist - A song','uploader':'Artist','duration':12,'url':url}]}));sys.exit(0)\n"
            "if 'youtube.com' in a[-1]: print('fixture source unavailable',file=sys.stderr);sys.exit(1)\n"
            "target=pathlib.Path(a[a.index('--output')+1]).parent/'download.wav'\n"
            "shutil.copyfile("+quoted+"[0],target)\n").toUtf8());file.close();QVERIFY(file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
        SettingsStore settings;settings.setLibraryFolder(dir.filePath("music"));settings.setYtDlpPath(executable);
        LibraryModel library(nullptr,dir.filePath("db"));Track t;t.title="A song";t.artist="Artist";t.duration=12000;t.source="Яндекс Музыка";t.sourceUrl="https://music.yandex.ru/track/12";
        const int id=library.repository().saveTrack(t);QVERIFY(library.repository().setRating(id,5));const int list=library.repository().createPlaylist("Keep");QVERIFY(library.repository().addToPlaylist(list,id));
        ImportManager importer(&library,&settings);DependencyManager tools(&settings);DownloadManager downloads(&settings,&importer,&tools,nullptr);AudioSources sources(&library,&downloads,&tools);
        sources.recoverTrack(id);QTRY_VERIFY_WITH_TIMEOUT(!sources.recovering(),15000);QVERIFY(!downloads.busy());
        const auto saved=library.repository().track(id);QVERIFY2(QFileInfo::exists(saved.filePath),qPrintable(sources.recoveryStatus()));QCOMPARE(saved.rating,5);
        QCOMPARE(library.repository().tracks({},list).first().id,id);QCOMPARE(library.repository().count(),1);QCOMPARE(downloads.succeededCount(),1);QCOMPARE(downloads.failedCount(),0);
        QCOMPARE(downloads.currentService(),QString());QCOMPARE(downloads.rowCount(),2);
#endif
    }
    void matchingTracksRejectsVersionsAndNormalizesArtistNames() {
        const QVariantMap target{{"artist","VOJ, Narvent"},{"title","Memory Reboot"},{"durationMs",209000}};
        QVariantMap candidate{{"artist","Narvent"},{"title","VØJ & Narvent - Memory Reboot (Official Music Video)"},{"durationMs",211000}};
        QCOMPARE(AudioSources::matchScore(target,candidate),100);
        candidate["title"]="VØJ & Narvent - Memory Reboot (Slowed Remix)";QVERIFY(AudioSources::matchScore(target,candidate)<=50);
        candidate["title"]="VOJ Narvent Memory Reboot";candidate["durationMs"]=300000;QVERIFY(AudioSources::matchScore(target,candidate)<=55);
    }
    void audioReactivityFollowsKicksAndSilence() {
        QAudioFormat format;format.setSampleRate(48000);format.setChannelCount(2);format.setSampleFormat(QAudioFormat::Float);
        AudioReactivity rhythm,steady;
        double maximumPulse=0,maximumBass=0,maximumEnergy=0;
        for(int tick=0;tick<400;++tick) {
            QByteArray kick(480*2*sizeof(float),Qt::Uninitialized),tone=kick;
            auto *k=reinterpret_cast<float*>(kick.data()),*s=reinterpret_cast<float*>(tone.data());
            for(int frame=0;frame<480;++frame) {
                const double time=(tick*480+frame)/48000.0;
                const double beat=std::fmod(time,.5);
                const double signal=beat<.08?.7*std::exp(-beat*25)*std::sin(time*2*3.141592653589793*65):0;
                // Opposite stereo phases must still have bass energy.
                k[frame*2]=float(signal);k[frame*2+1]=float(-signal);
                s[frame*2]=s[frame*2+1]=float(.2*std::sin(time*2*3.141592653589793*65));
            }
            rhythm.process(QAudioBuffer(kick,format));steady.process(QAudioBuffer(tone,format));
            maximumPulse=qMax(maximumPulse,rhythm.state().pulse);maximumBass=qMax(maximumBass,rhythm.state().bass);maximumEnergy=qMax(maximumEnergy,rhythm.state().energy);
        }
        QVERIFY(rhythm.state().onsets>=6);QVERIFY(rhythm.state().onsets<=8);
        QVERIFY(steady.state().onsets<=1);QVERIFY(maximumPulse>.35);QVERIFY(maximumBass>.3);QVERIFY(maximumEnergy>.3);
        QByteArray silence(480*2*sizeof(float),0);
        for(int i=0;i<200;++i)rhythm.process(QAudioBuffer(silence,format));
        QVERIFY(rhythm.state().energy<.001);QVERIFY(rhythm.state().bass<.001);QVERIFY(rhythm.state().pulse<.001);
        const auto beats=rhythm.state().onsets;
        for(int i=0;i<100;++i)rhythm.process(QAudioBuffer(silence,format));QCOMPARE(rhythm.state().onsets,beats);
        rhythm.reset();QCOMPARE(rhythm.state().onsets,quint64(0));QCOMPARE(rhythm.state().energy,0.0);
    }
    void rejectedBrowserNavigationRecovers_data() {
        QTest::addColumn<QString>("service");QTest::addColumn<QString>("url");QTest::addColumn<QString>("error");QTest::addColumn<bool>("withoutEvents");
        QTest::newRow("VK-detached-frame")<<"vk"<<"https://vk.ru/audios99?section=all"<<"Not attached to an active page"<<false;
        QTest::newRow("Hitmo-rejected-command")<<"hitmo"<<"https://rus.hitmos.fm/"<<"Navigation failed"<<false;
        QTest::newRow("Zaycev-Russian-timeout")<<"zaycev"<<"https://zaycev.net/"<<"Таймаут команды."<<true;
        QTest::newRow("Yandex-ready-probe")<<"yandex"<<"https://music.yandex.ru/collection"<<"Command timed out"<<true;
    }
    void rejectedBrowserNavigationRecovers() {
#ifndef Q_OS_WIN
        QFETCH(QString,service);QFETCH(QString,url);QFETCH(QString,error);QFETCH(bool,withoutEvents);
        QTemporaryDir dir;const auto executable=dir.filePath("browser");QFile file(executable);
        QVERIFY(file.open(QIODevice::WriteOnly));file.write("#!/bin/sh\nexec sleep 30\n");file.close();QVERIFY(file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
        DevToolsTransport network;BrowserProtocol protocol;protocol.navigationFailures=1;protocol.navigationError=error;protocol.fallbackWithoutEvents=withoutEvents;
        BrowserSession session(nullptr,&network,&protocol,[]{return 9222;});QSignalSpy loaded(&session,&BrowserSession::loaded),failed(&session,&BrowserSession::failed);
        session.open(executable,service,QUrl(url),{},true);
        QTRY_VERIFY_WITH_TIMEOUT(!loaded.isEmpty(),4000);QVERIFY(failed.isEmpty());QCOMPARE(loaded.last()[0].toUrl(),QUrl(url));QVERIFY(loaded.last()[1].toBool());
        const auto diagnostic=session.protocolDiagnostic();QVERIFY(diagnostic.value("navigationFallback").toBool());QVERIFY(diagnostic.value("navigationReady").toBool());
        QCOMPARE(diagnostic.value("code").toInt(),-32000);QCOMPARE(diagnostic.value("error").toString(),CdpClient::errorCategory(error));
        QVERIFY(!QJsonDocument(diagnostic).toJson().contains("section=all"));
        // A stale about:blank error arrives after a new operation has opened.
        session.hide();const int oldNavigation=protocol.next;
        session.open(executable,service,QUrl(url),{},true);QTRY_VERIFY(loaded.count()>=2);
        emit protocol.commandError(oldNavigation,"Not attached to an active page",-32000);QVERIFY(failed.isEmpty());
        // Missing JS context at startup is transient; retry once the page exists.
        protocol.navigationFailures=1;protocol.fallbackContextFailures=1;
        session.open(executable,service,QUrl(url),{},true);const int previousLoads=loaded.count();
        QTRY_VERIFY_WITH_TIMEOUT(loaded.count()>previousLoads,4000);QVERIFY(failed.isEmpty());
        session.stop();
#endif
    }
    void browserProtocolClassifiesErrorsWithoutPrivateMessages() {
        QCOMPARE(CdpClient::errorCategory("Таймаут команды."),QString("timeout"));
        QCOMPARE(CdpClient::errorCategory("Command timed out"),QString("timeout"));
        QCOMPARE(CdpClient::errorCategory("Cannot navigate to invalid URL: https://private.example/token"),QString("invalid-url"));
        QCOMPARE(CdpClient::errorCategory("Execution context was destroyed"),QString("context-changed"));
        QCOMPARE(CdpClient::errorCategory("Frame not found"),QString("frame-unavailable"));
    }
    void browserNavigationFailureStopsWithoutRetryLoop() {
#ifndef Q_OS_WIN
        QTemporaryDir dir;const auto executable=dir.filePath("browser");QFile file(executable);
        QVERIFY(file.open(QIODevice::WriteOnly));file.write("#!/bin/sh\nexec sleep 30\n");file.close();QVERIFY(file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
        DevToolsTransport network;BrowserProtocol protocol;protocol.navigationFailures=1;protocol.fallbackException=true;
        BrowserSession session(nullptr,&network,&protocol,[]{return 9222;});QSignalSpy loaded(&session,&BrowserSession::loaded),failed(&session,&BrowserSession::failed);
        session.open(executable,"hitmo",QUrl("https://rus.hitmos.fm/"),{},true);
        QTRY_COMPARE_WITH_TIMEOUT(failed.count(),1,3000);QVERIFY(loaded.isEmpty());QVERIFY(!failed.first()[0].toString().contains("private-error"));
        const auto commands=protocol.commands.size();QTest::qWait(400);QCOMPARE(failed.count(),1);QCOMPARE(protocol.commands.size(),commands);
#endif
    }
    void metadataAlbumAndCoverEditsPreserveIdentityAndFiles() {
        QTemporaryDir dir;LibraryModel library(nullptr,dir.filePath("library.db"));
        Track t;t.title="Original";t.artist="Artist";t.album="Old album";t.source="Яндекс Музыка";t.sourceUrl="https://music.yandex.ru/track/123";
        const int id=library.repository().saveTrack(t);QVERIFY(id>0);t.title="Second";t.sourceUrl="https://vk.ru/audio99_2";const int second=library.repository().saveTrack(t);
        const int list=library.repository().createPlaylist("Keep");QVERIFY(library.repository().addToPlaylist(list,id));QVERIFY(library.repository().setRating(id,5));QVERIFY(library.repository().recordPlay(id));
        const auto imagePath=dir.filePath("cover with spaces.png");QImage image(40,40,QImage::Format_RGB32);image.fill(Qt::cyan);QVERIFY(image.save(imagePath));
        QVERIFY(library.setTrackCover(id,QUrl::fromLocalFile(imagePath)));const auto cover=library.repository().track(id).coverPath;
        QVERIFY(cover!=imagePath);QVERIFY(QFileInfo::exists(cover));QVERIFY(QFile::remove(imagePath));
        QVERIFY(library.attachFile(id,wav(dir.filePath("audio.wav"))));QCOMPARE(library.repository().track(id).coverPath,cover);
        QVERIFY(library.updateMetadata(id,"Edited","New artist","Old album"));
        QVERIFY(library.setAlbumTracks("Old album","New album",{id}));
        const auto edited=library.repository().track(id);QCOMPARE(edited.title,QString("Edited"));QCOMPARE(edited.artist,QString("New artist"));QCOMPARE(edited.album,QString("New album"));
        QCOMPARE(edited.sourceUrl,QString("https://music.yandex.ru/track/123"));QCOMPARE(edited.rating,5);QCOMPARE(edited.playCount,1);QCOMPARE(edited.coverPath,cover);
        QVERIFY(QFileInfo::exists(edited.filePath));QCOMPARE(library.repository().tracks({},list).first().id,id);QVERIFY(library.repository().track(second).album.isEmpty());
        QVERIFY(!library.setAlbumTracks("New album","Invalid",{99999}));QCOMPARE(library.repository().track(id).album,QString("New album"));
        Track reimport;reimport.title="Original";reimport.artist="Artist";reimport.source="Яндекс Музыка";reimport.sourceUrl="https://music.yandex.ru/track/123";
        QCOMPARE(library.repository().saveTrack(reimport),id);QCOMPARE(library.repository().count(),2);QCOMPARE(library.repository().track(id).title,QString("Edited"));
    }
    void publicSourceSearchDownloadsIntoExistingTrack_data() {
        QTest::addColumn<QString>("source");QTest::addColumn<QString>("sourceUrl");QTest::addColumn<QString>("searchUrl");
        QTest::newRow("Hitmo")<<"Hitmo"<<"https://rus.hitmos.fm/song/123"<<"https://rus.hitmos.fm/search?q=Artist+A+song";
        QTest::newRow("Zaycev")<<"Zaycev.net"<<"https://zaycev.net/pages/123/456.shtml"<<"https://zaycev.net/search?query_search=Artist+A+song";
    }
    void publicSourceSearchDownloadsIntoExistingTrack() {
#ifndef Q_OS_WIN
        QFETCH(QString,source);QFETCH(QString,sourceUrl);QFETCH(QString,searchUrl);
        QTemporaryDir dir;const auto executable=dir.filePath("brave");QFile file(executable);
        QVERIFY(file.open(QIODevice::WriteOnly));file.write("#!/bin/sh\nexec sleep 30\n");file.close();QVERIFY(file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
        const auto oldPath=qgetenv("PATH");const auto restore=qScopeGuard([&]{qputenv("PATH",oldPath);});qputenv("PATH",dir.path().toUtf8()+":"+oldPath);
        SettingsStore settings;settings.setLibraryFolder(dir.filePath("music"));LibraryModel library(nullptr,dir.filePath("library.db"));
        Track track;track.title="A song";track.artist="Artist";track.source="Яндекс Музыка";track.sourceUrl="https://music.yandex.ru/track/123";track.duration=12000;
        const int id=library.repository().saveTrack(track);QVERIFY(id>0);QVERIFY(library.repository().setRating(id,5));
        const int list=library.repository().createPlaylist("Keep");QVERIFY(library.repository().addToPlaylist(list,id));
        ImportManager importer(&library,&settings);WebLoginManager web(&settings);BrowserAuthManager auth(&settings,&web);AudioTransport transport;
        QFile audio(wav(dir.filePath("audio.wav")));QVERIFY(audio.open(QIODevice::ReadOnly));transport.payload=audio.readAll();
        DownloadManager downloads(&settings,&importer,nullptr,&auth,nullptr,&transport);DevToolsTransport network;BrowserProtocol protocol;
        protocol.publicSearchNavigation=searchUrl;protocol.navigationFailures=1;
        transport.status=403;
        QVERIFY(downloads.enqueueAlternative(id,{{"url",track.sourceUrl},{"mediaUrl","https://cdn.example/denied.wav"}},"original"));
        QTRY_VERIFY_WITH_TIMEOUT(!downloads.busy(),5000);QCOMPARE(downloads.failedCount(),1);QCOMPARE(downloads.firstFailedTrack(),id);
        QFile diagnostic(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/diagnostics/downloads.json");
        QVERIFY(diagnostic.open(QIODevice::ReadOnly));const auto report=diagnostic.readAll();
        const auto record=QJsonDocument::fromJson(report).array().last().toObject();
        QCOMPARE(record.value("category").toString(),QString("access-denied"));
        QVERIFY(!report.contains("cdn.example"));QVERIFY(!report.contains("sourceUrl"));
        QVERIFY(!downloads.data(downloads.index(0),DownloadManager::ErrorRole).toString().isEmpty());transport.status=200;
        protocol.publicFixture=[sourceUrl](const QJsonObject &options) {
            if(options.value("resolve").toBool())return QJsonObject{{"mediaUrl","https://cdn.example/stream?id=42"},{"mediaVerified",true}};
            if(!options.value("submitted").toBool())return QJsonObject{{"submitted",true}};
            return QJsonObject{{"submitted",true},{"tracks",QJsonArray{QJsonObject{{"title","A song"},{"artist","Artist"},{"durationMs",12000},{"url",sourceUrl}}}}};
        };
        AudioSources sources(&library,&downloads,nullptr,nullptr,[&](QObject *parent){return new BrowserSession(parent,&network,&protocol,[]{return 9222;});});
        QSignalSpy queued(&sources,&AudioSources::queued);sources.openTrack(id);sources.search(source);QTRY_VERIFY_WITH_TIMEOUT(!sources.busy(),6000);
        QCOMPARE(sources.results().size(),1);QCOMPARE(sources.results()[0].toMap().value("score").toInt(),100);
        QUrl searched;
        for (const auto& visited : protocol.navigationHistory) {
            const QUrl url(visited);
            if (url.host() == QUrl(searchUrl).host() && url.path() == QUrl(searchUrl).path())
                searched = url;
        }
        QCOMPARE(searched.path(),QUrl(searchUrl).path());
        QCOMPARE(QUrlQuery(searched).queryItemValue(source=="Hitmo"?"q":"query_search").toCaseFolded(), QString("artist a song"));
        sources.choose(0);QTRY_COMPARE_WITH_TIMEOUT(queued.count(),1,5000);QTRY_VERIFY_WITH_TIMEOUT(!downloads.busy(),6000);
        QCOMPARE(downloads.succeededCount(),1);QCOMPARE(downloads.failedCount(),0);QCOMPARE(library.repository().count(),1);
        QCOMPARE(downloads.firstFailedTrack(),0);QCOMPARE(downloads.data(downloads.index(0),DownloadManager::StatusRole).toString(),QString("recovered"));
        const auto saved=library.repository().track(id);QVERIFY(QFileInfo::exists(saved.filePath));QCOMPARE(saved.sourceUrl,track.sourceUrl);QCOMPARE(saved.rating,5);
        QCOMPARE(library.repository().tracks({},list).first().id,id);QCOMPARE(downloads.trackState(id).value("service").toString(),source);
        QVERIFY(!transport.lastRequest.hasRawHeader("Cookie"));
        QVERIFY(!downloads.enqueueAlternative(id,{{"url","https://youtube.com.evil.test/watch?v=1"}},"original"));
#endif
    }
    void vkIncomplete1300TrackCollectionIsNotSavedAs45() {
#ifndef Q_OS_WIN
        QTemporaryDir dir;const auto executable=dir.filePath("brave");QFile file(executable);
        QVERIFY(file.open(QIODevice::WriteOnly));file.write("#!/bin/sh\nexec sleep 30\n");file.close();QVERIFY(file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
        const auto oldPath=qgetenv("PATH");const auto restore=qScopeGuard([&]{qputenv("PATH",oldPath);});qputenv("PATH",dir.path().toUtf8()+":"+oldPath);
        SettingsStore settings;DevToolsTransport network;BrowserProtocol protocol;QJsonArray tracks;
        for(int i=1;i<=45;++i)tracks.append(QJsonObject{{"title","Track "+QString::number(i)},{"url","https://vk.ru/audio99_"+QString::number(i)}});
        protocol.harvestOverride={{"expectedCount",1300},{"atBottom",true},{"tracks",tracks},{"loading",false}};
        WebLoginManager web(&settings,nullptr,[&](QObject *parent){return new BrowserSession(parent,&network,&protocol,[]{return 9222;});});
        LibraryModel library(nullptr,dir.filePath("library.db"));LibraryExchange exchange(&library,&settings,nullptr,nullptr,&web);
        exchange.importUrl("https://vk.ru/audios99");QTRY_VERIFY_WITH_TIMEOUT(!exchange.busy(),14000);
        QCOMPARE(library.repository().count(),0);QVERIFY(exchange.status().contains("45 из 1300"));QVERIFY(library.repository().playlists().isEmpty());web.logout("vk");
#endif
    }
    void yandexPayloadPreservesIdsOrderAndRichMetadata() {
        const auto parsed=ServicePayload::yandex(QJsonObject{{"trackCount",3},{"playlistUuid","lk.test"},{"tracks",QJsonArray{
            QJsonObject{{"id",23}},QJsonObject{{"id","11"},{"title","Song"},{"durationMs",123000},
                {"artists",QJsonArray{QJsonObject{{"name","Artist"}}}},{"albums",QJsonArray{QJsonObject{{"title","Album"}}}},
                {"secret","must-not-be-retained"}},QJsonObject{{"id",23}}}}});
        QVERIFY(parsed.fullPlaylist);QCOMPARE(parsed.total,3);QCOMPARE(parsed.order.size(),2);
        QCOMPARE(parsed.order.first(),QString("https://music.yandex.ru/track/23"));
        QCOMPARE(parsed.tracks.size(),1);QCOMPARE(parsed.tracks[0].artist,QString("Artist"));
        QCOMPARE(parsed.tracks[0].album,QString("Album"));QCOMPARE(parsed.tracks[0].duration,123000);
        QVERIFY(!parsed.tracks[0].webData.contains("secret"));
        QVERIFY(BrowserSession::isCollectionApi(QUrl("https://api.music.yandex.ru/playlist/lk.test")));
        QVERIFY(!BrowserSession::isCollectionApi(QUrl("https://api.music.yandex.ru.evil.test/tracks")));
        QVERIFY(!BrowserSession::isCollectionApi(QUrl("https://api.music.yandex.ru/account")));
        QVERIFY(BrowserSession::isCollectionApi(QUrl("https://api.music.yandex.net/users/99/playlists/3")));
        QVERIFY(BrowserSession::jsonFetchScript(QUrl("https://api.music.yandex.ru.evil.test/tracks")).isEmpty());
        QCOMPARE(ServicePayload::canonicalYandexTrackUrl(QUrl("https://music.yandex.ru/album/77/track/23?from=playlist")),
                 QString("https://music.yandex.ru/track/23"));
        QVERIFY(ServicePayload::canonicalYandexTrackUrl(QUrl("https://music.yandex.ru.evil.test/track/23")).isEmpty());
    }
    void browserSeedsCookiesAndNavigatesWithoutRendererUserAgent() {
#ifndef Q_OS_WIN
        QTemporaryDir dir;const auto executable=dir.filePath("brave");QFile file(executable);
        QVERIFY(file.open(QIODevice::WriteOnly));file.write("#!/bin/sh\nexec sleep 30\n");file.close();
        QVERIFY(file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
        DevToolsTransport network;BrowserProtocol protocol;protocol.rejectedCookie="optional";protocol.stallBrowserVersion=true;
        BrowserSession session(nullptr,&network,&protocol,[]{return 9222;});
        QSignalSpy loaded(&session,&BrowserSession::loaded),failed(&session,&BrowserSession::failed);
        const QJsonArray cookies{QJsonObject{{"name","remixsid6"},{"domain",".vk.ru"},{"value","private-session"}},
            QJsonObject{{"name","optional"},{"domain",".vk.ru"},{"value","private-optional"}}};
        session.open(executable,"vk",QUrl("https://vk.ru/audios99"),cookies,true,true);
        QTRY_VERIFY_WITH_TIMEOUT(!loaded.isEmpty(),4000);QVERIFY(failed.isEmpty());
        bool navigated=false;
        for(const auto &call:protocol.commands) {
            if(call.first=="Page.navigate")navigated=true;
            QVERIFY(call.second.value("expression").toString()!="navigator.userAgent");
        }
        QVERIFY(navigated);
        QCOMPARE(session.protocolDiagnostic().value("cookiesAccepted").toInt(),1);
        QCOMPARE(session.protocolDiagnostic().value("cookiesRejected").toInt(),1);
        session.stop();protocol.rejectedCookie="remixsid6";
        session.open(executable,"vk",QUrl("https://vk.ru/audios99"),cookies,true,true);
        QTRY_COMPARE_WITH_TIMEOUT(failed.count(),1,4000);
        QVERIFY(failed[0][0].toString().contains("Network.setCookie"));
        QVERIFY(!QJsonDocument(session.protocolDiagnostic()).toJson().contains("private-"));
        QVERIFY(!failed[0][0].toString().contains("private-"));
#endif
    }
    void yandexCollectionReadsAll24InsteadOfEightPreviewRows_data() {
        QTest::addColumn<int>("apiStatus");
        QTest::addColumn<int>("trackCount");
        QTest::addColumn<bool>("passiveBody");
        QTest::newRow("full-collection")<<200<<24<<true;
        QTest::newRow("multiple-metadata-batches")<<200<<225<<true;
        QTest::newRow("session-rejected-no-partial-import")<<403<<24<<true;
        QTest::newRow("response-body-unavailable-request-explicitly")<<200<<24<<false;
    }
    void yandexCollectionReadsAll24InsteadOfEightPreviewRows() {
#ifndef Q_OS_WIN
        QFETCH(int,apiStatus);
        QFETCH(int,trackCount);
        QFETCH(bool,passiveBody);
        QTemporaryDir dir;const auto executable=dir.filePath("brave");QFile file(executable);
        QVERIFY(file.open(QIODevice::WriteOnly));file.write("#!/bin/sh\nexec sleep 30\n");file.close();
        QVERIFY(file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
        const auto oldPath=qgetenv("PATH");const auto restore=qScopeGuard([&]{qputenv("PATH",oldPath);});
        qputenv("PATH",dir.path().toUtf8()+":"+oldPath);
        SettingsStore settings;DevToolsTransport network;BrowserProtocol protocol;
        protocol.harvestOverride={{"atBottom",true},{"tracks",QJsonArray{}}};
        const auto rich=[](int id){return QJsonObject{{"id",QString::number(id)},{"title","Song "+QString::number(id)},
            {"artists",QJsonArray{QJsonObject{{"name","Artist"}}}},{"durationMs",120000}};};
        QJsonArray preview,ids;for(int i=trackCount;i>=1;--i){ids.append(QJsonObject{{"id",i},{"albumId",77}});if(i>trackCount-8)preview.append(rich(i));}
        protocol.responseBody={{"summary",QJsonObject{{"count",trackCount}}},{"tracks",preview},
            {"playlist",QJsonObject{{"playlistUuid","lk.test"},{"title","Liked songs"}}}};
        protocol.apiFixture=[&](const QJsonObject &args) {
            if(apiStatus!=200)return QJsonObject{{"status",apiStatus},{"payload",QJsonObject{{"error","private-account-message"}}}};
            if(QUrl(args.value("url").toString()).path().endsWith("/playlist-with-likes"))
                return QJsonObject{{"status",200},{"payload",protocol.responseBody}};
            if(QUrl(args.value("url").toString()).path().startsWith("/playlist/"))
                return QJsonObject{{"status",200},{"payload",QJsonObject{{"playlistUuid","lk.test"},{"trackCount",trackCount},{"tracks",ids}}}};
            QJsonArray hydrated;for(const auto &id:args.value("ids").toArray())hydrated.append(rich(id.toString().toInt()));
            return QJsonObject{{"status",200},{"payload",hydrated}};
        };
        WebLoginManager web(&settings,nullptr,[&](QObject *parent){return new BrowserSession(parent,&network,&protocol,[]{return 9222;});});
        LibraryModel library(nullptr,dir.filePath("library.db"));LibraryExchange exchange(&library,&settings,nullptr,nullptr,&web);
        QSignalSpy completed(&exchange,&LibraryExchange::completed);exchange.importUrl("https://music.yandex.ru/collection/tracks");
        QTRY_VERIFY_WITH_TIMEOUT(protocol.isConnected(),4000);
        QTRY_VERIFY_WITH_TIMEOUT([&]{for(const auto &call:protocol.commands)if(call.first=="Page.navigate")return true;return false;}(),4000);
        QTest::qWait(20);
        emit protocol.eventReceived("Network.requestWillBeSent",{{"requestId","preview"},{"request",QJsonObject{
            {"url","https://api.music.yandex.ru/landing-blocks/collection/playlist-with-likes?count=8"},
            {"headers",QJsonObject{{"Authorization","private-token"}}}}}});
        if(passiveBody)emit protocol.eventReceived("Network.loadingFinished",{{"requestId","preview"},{"encodedDataLength",1000}});
        QTRY_VERIFY_WITH_TIMEOUT(!exchange.busy(),10000);
        if(apiStatus!=200){QVERIFY(completed.isEmpty());QCOMPARE(library.repository().count(),0);
            QVERIFY(exchange.status().contains("HTTP 403"));QVERIFY(!exchange.status().contains("private-"));web.logout("yandex");return;}
        QCOMPARE(completed.count(),1);QCOMPARE(library.repository().count(),trackCount);
        const auto tracks=library.repository().tracks({},completed[0][0].toInt());
        QCOMPARE(tracks.first().title,QString("Song ")+QString::number(trackCount));QCOMPARE(tracks.last().title,QString("Song 1"));
        QCOMPARE(tracks.last().artist,QString("Artist"));
        bool correctRoute=false,hydrated=false;
        for(const auto &call:protocol.commands){
            if(call.first=="Page.navigate" && call.second.value("url").toString()=="https://music.yandex.ru/collection")correctRoute=true;
            if(call.first=="Runtime.evaluate" && call.second.value("expression").toString().contains("o.body.append('trackIds',id)"))hydrated=true;
        }
        QVERIFY(correctRoute);QVERIFY(hydrated);web.logout("yandex");
#endif
    }
    void yandexLargePlaylistImport_data()
    {
        QTest::addColumn<QString>("scenario");
        for (const auto& scenario : QStringList{"nested-manifest", "paged-shrinking-count", "metadata-retry",
                                               "repeated-page", "metadata-permanently-missing", "cancel-retry",
                                               "legacy-manifest", "api-unavailable-dom198", "reimport-198", "out-of-order-page"})
            QTest::newRow(qPrintable(scenario)) << scenario;
    }
    void yandexLargePlaylistImport()
    {
#ifdef Q_OS_WIN
        QSKIP("Fixture browser uses a POSIX launcher");
#else
        QFETCH(QString, scenario);
        QTemporaryDir dir;
        const auto executable = dir.filePath("brave");
        QFile browser(executable);
        QVERIFY(browser.open(QIODevice::WriteOnly));
        browser.write("#!/bin/sh\nexec sleep 30\n");
        browser.close();
        QVERIFY(browser.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
        const auto previousPath = qgetenv("PATH");
        const auto restore = qScopeGuard([&] { qputenv("PATH", previousPath); });
        qputenv("PATH", dir.path().toUtf8() + ':' + previousPath);
        constexpr int total = 2037;
        const QString uuid = "lk.b964a04c-643d-49f4-9c60-927c0552288b";
        const auto rich = [](int id) {
            return QJsonObject{{"id", QString::number(id)}, {"title", "Song " + QString::number(id)},
                               {"artists", QJsonArray{QJsonObject{{"name", "Artist"}}}}, {"durationMs", 120000}};
        };
        QJsonArray dom;
        for (int id = total; id > total - 198; --id)
            dom.append(QJsonObject{{"title", "Preview " + QString::number(id)},
                                   {"url", "https://music.yandex.ru/track/" + QString::number(id)}});
        SettingsStore settings;
        DevToolsTransport transport;
        BrowserProtocol protocol;
        protocol.harvestOverride = {{"atBottom", true}, {"complete", true}, {"loading", false}, {"tracks", dom}};
        int metadataCalls = 0, playlistCalls = 0, legacyCalls = 0;
        int largestBatch = 0;
        protocol.apiFixture = [&](const QJsonObject& args) {
            const QUrl url(args.value("url").toString());
            if (url.path() == "/tracks") {
                ++metadataCalls;
                const auto ids = args.value("ids").toArray();
                largestBatch = qMax(largestBatch, int(ids.size()));
                if ((scenario == "metadata-retry" && metadataCalls == 1) || scenario == "cancel-retry")
                    return QJsonObject{{"status", 429}};
                QJsonArray entries;
                for (const auto& value : ids) {
                    const int id = value.toString().toInt();
                    if (scenario == "metadata-permanently-missing" && id == 17)
                        continue;
                    entries.append(rich(id));
                }
                return QJsonObject{{"status", 200}, {"payload", entries}};
            }
            if (scenario == "api-unavailable-dom198")
                return QJsonObject{{"status", 0}};
            const bool legacy = url.path().startsWith("/users/");
            if (legacy) ++legacyCalls;
            else ++playlistCalls;
            QJsonArray ids;
            if (scenario == "paged-shrinking-count" || scenario == "out-of-order-page" || scenario == "repeated-page" ||
                (scenario == "legacy-manifest" && !legacy)) {
                const int page = scenario == "repeated-page" ? 0 : QUrlQuery(url).queryItemValue("page").toInt();
                for (int index = page * 200; index < qMin(total, (page + 1) * 200); ++index)
                    ids.append(QJsonObject{{"id", total - index}});
                QJsonObject playlist{{"playlistUuid", uuid}, {"trackCount", page == 0 ? total : 198}, {"tracks", ids}};
                if (scenario == "legacy-manifest") {
                    playlist["owner"] = QJsonObject{{"uid", "99"}};
                    playlist["kind"] = "3";
                }
                return QJsonObject{{"status", 200}, {"payload", QJsonObject{{"result", playlist}}}};
            }
            for (int id = total; id > 0; --id)
                ids.append(id % 2 ? QJsonValue(QString::number(id) + ":77") : QJsonValue(QJsonObject{{"id", id}}));
            QJsonArray preview;
            for (int id = total; id > total - 198; --id)
                preview.append(rich(id));
            return QJsonObject{{"status", 200}, {"payload", QJsonObject{{"result", QJsonObject{
                {"pager", QJsonObject{{"total", total}}},
                {"playlist", QJsonObject{{"playlistUuid", uuid}, {"title", "Large playlist"},
                    {"trackCount", 198}, {"trackIds", ids}, {"tracks", preview}}}}}}}};
        };
        WebLoginManager web(&settings, nullptr, [&](QObject* parent) {
            return new BrowserSession(parent, &transport, &protocol, [] { return 9222; });
        });
        LibraryModel library(nullptr, dir.filePath("large.db"));
        int savedId = 0;
        if (scenario == "reimport-198") {
            for (int id = 1; id <= 198; ++id) {
                Track old;
                old.title = "Song " + QString::number(id);
                old.artist = "Artist";
                old.source = "Яндекс Музыка";
                old.sourceUrl = "https://music.yandex.ru/album/77/track/" + QString::number(id) + "?from=playlist";
                if (id == 1) old.filePath = wav(dir.filePath("saved.wav"));
                const int local = library.repository().saveTrack(old);
                QVERIFY(local > 0);
                if (id == 17) savedId = local;
            }
            QVERIFY(library.repository().setRating(savedId, 5));
            QVERIFY(library.repository().recordListen(savedId, 3000, false));
        }
        LibraryExchange exchange(&library, &settings, nullptr, nullptr, &web);
        QSignalSpy completed(&exchange, &LibraryExchange::completed);
        QSignalSpy collected(&web, &WebLoginManager::collectionReady);
        const QString link = "https://music.yandex.ru/playlists/" + uuid +
                             "?ref_id=fixture&utm_source=mobile_ios&utm_medium=share_link_tg";
        exchange.importUrl(link);
        if (scenario == "out-of-order-page") {
            QTRY_VERIFY_WITH_TIMEOUT(protocol.isConnected(), 4000);
            QTRY_VERIFY_WITH_TIMEOUT(!protocol.navigationUrl.isEmpty(), 4000);
            QJsonArray page;
            for (int index = 200; index < 400; ++index)
                page.append(QJsonObject{{"id", total - index}});
            protocol.responseBody = {{"result", QJsonObject{{"playlistUuid", uuid}, {"trackCount", 198}, {"tracks", page}}}};
            emit protocol.eventReceived("Network.requestWillBeSent", {{"requestId", "page-first"},
                {"request", QJsonObject{{"url", "https://api.music.yandex.net/playlist/" + uuid + "?page=1"}}}});
            emit protocol.eventReceived("Network.loadingFinished", {{"requestId", "page-first"}, {"encodedDataLength", 4096}});
        }
        if (scenario == "cancel-retry") {
            QTRY_VERIFY_WITH_TIMEOUT(metadataCalls > 0, 8000);
            const int attemptsAtCancellation = metadataCalls;
            exchange.cancel();
            QTest::qWait(1800);
            QVERIFY(!exchange.busy());
            QCOMPARE(metadataCalls, attemptsAtCancellation);
            QCOMPARE(library.repository().count(), 0);
            QVERIFY(completed.isEmpty());
            return;
        }
        QTRY_VERIFY_WITH_TIMEOUT(!exchange.busy(), 12000);
        if (scenario == "repeated-page" || scenario == "metadata-permanently-missing" ||
            scenario == "api-unavailable-dom198") {
            QVERIFY(completed.isEmpty());
            QCOMPARE(library.repository().count(), 0);
            QVERIFY(library.repository().playlists().isEmpty());
            QVERIFY(exchange.status().contains("Неполный список"));
            return;
        }
        QCOMPARE(completed.count(), 1);
        QCOMPARE(library.repository().count(), total);
        const auto tracks = library.repository().tracks({}, completed[0][0].toInt());
        QCOMPARE(tracks.size(), total);
        QCOMPARE(tracks.first().title, QString("Song 2037"));
        QCOMPARE(tracks.last().title, QString("Song 1"));
        QVERIFY(largestBatch <= 200);
        QVERIFY(metadataCalls >= 10);
        if (scenario == "reimport-198") {
            QCOMPARE(tracks[total - 17].id, savedId);
            QCOMPARE(library.repository().track(savedId).rating, 5);
            QCOMPARE(library.repository().track(savedId).listenMs, 3000);
            QVERIFY(QFileInfo::exists(tracks.last().filePath));
        }
        if (scenario == "paged-shrinking-count") QCOMPARE(playlistCalls, 11);
        if (scenario == "legacy-manifest") QCOMPARE(legacyCalls, 1);
        // API responses from a cancelled/completed operation cannot import another partial list.
        emit web.collectionReady({}, {}, "late-fixture");
        QCOMPARE(completed.count(), 1);
#endif
    }
    void unrecognizedWebPageReportsStructureWithoutSessionData() {
#ifndef Q_OS_WIN
        QTemporaryDir dir;const QString browser=dir.filePath("brave");QFile executable(browser);
        QVERIFY(executable.open(QIODevice::WriteOnly));executable.write("#!/bin/sh\nexec sleep 30\n");executable.close();QVERIFY(executable.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
        const auto oldPath=qgetenv("PATH");const auto restore=qScopeGuard([&]{qputenv("PATH",oldPath);});qputenv("PATH",dir.path().toUtf8()+":"+oldPath);
        SettingsStore settings;DevToolsTransport network;BrowserProtocol protocol;
        protocol.harvestOverride={{"url","https://name:private-password@vk.ru/audios99?access_key=private-access-key#private-fragment"},{"challenge",true},{"tracks",QJsonArray{}},
            {"diagnostics",QJsonObject{{"version",2},{"modernRows",8},{"recognized",0},{"readyState","complete"},{"cookies","private-cookie"},{"html","private-html"}}}};
        WebLoginManager web(&settings,nullptr,[&](QObject *parent){return new BrowserSession(parent,&network,&protocol,[]{return 9222;});});
        LibraryModel library(nullptr,dir.filePath("library.db"));LibraryExchange exchange(&library,&settings,nullptr,nullptr,&web);
        exchange.importUrl("https://vk.ru/audios99");QTRY_VERIFY_WITH_TIMEOUT(!exchange.busy(),4000);
        QCOMPARE(library.repository().count(),0);QVERIFY(library.repository().playlists().isEmpty());QVERIFY(exchange.status().contains("проверку безопасности"));
        QFile report(web.importReport());QVERIFY(report.open(QIODevice::ReadOnly));const auto bytes=report.readAll();
        QVERIFY(!bytes.contains("private-"));const auto diagnostic=QJsonDocument::fromJson(bytes).object();
        QCOMPARE(diagnostic.value("page").toString(),QString("https://vk.ru/audios99"));QCOMPARE(diagnostic.value("pageStructure").toObject().value("modernRows").toInt(),8);
        QVERIFY(!(report.permissions()&(QFile::ReadOther|QFile::WriteOther|QFile::ReadGroup|QFile::WriteGroup)));web.logout("vk");
#endif
    }
    void localBrowserProfilesAndCookiesUseExistingLogin() {
        QTemporaryDir dir;const auto profile=browserProfile(dir.path());QVERIFY(!profile.isEmpty());
        SettingsStore settings;BrowserCookieImporter importer(&settings,nullptr,dir.path());
        const auto profiles=importer.availableProfiles();QCOMPARE(profiles.size(),1);
        QCOMPARE(profiles[0].toMap().value("id").toString(),profile);
        QVERIFY(profiles[0].toMap().value("label").toString().contains("Already signed in"));
        QSignalSpy completed(&importer,&BrowserCookieImporter::finished);importer.read("VK",profile);
        QTRY_COMPARE_WITH_TIMEOUT(completed.count(),1,5000);QVERIFY(completed[0][3].toString().isEmpty());
        const auto cookies=qvariant_cast<QJsonArray>(completed[0][2]);QCOMPARE(cookies.size(),1);
        QCOMPARE(cookies[0].toObject().value("name").toString(),QString("remixsid6"));
        importer.read("yandex",profile);QTRY_COMPARE_WITH_TIMEOUT(completed.count(),2,5000);
        QVERIFY(completed[1][3].toString().contains("нет действующей сессии"));QVERIFY(qvariant_cast<QJsonArray>(completed[1][2]).isEmpty());
    }
    void existingBrowserSessionImportsAndDownloadsWithoutNewLogin() {
#ifndef Q_OS_WIN
        QTemporaryDir dir;const auto profile=browserProfile(dir.path());QVERIFY(!profile.isEmpty());
        const QString browser=dir.filePath("brave");QFile executable(browser);QVERIFY(executable.open(QIODevice::WriteOnly));
        executable.write("#!/bin/sh\nexec sleep 30\n");executable.close();QVERIFY(executable.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
        const auto oldPath=qgetenv("PATH");const auto restore=qScopeGuard([&]{qputenv("PATH",oldPath);});
        qputenv("PATH",dir.path().toUtf8()+":"+oldPath);
        SettingsStore settings;settings.setLibraryFolder(dir.filePath("music"));BrowserCookieImporter cookies(&settings,nullptr,dir.path());
        DevToolsTransport network;BrowserProtocol protocol;protocol.navigationFailures=1;BrowserSession *session=nullptr;
        WebLoginManager web(&settings,nullptr,[&](QObject *parent){session=new BrowserSession(parent,&network,&protocol,[]{return 9222;});return session;},&cookies);
        LibraryModel library(nullptr,dir.filePath("library.db"));ImportManager importer(&library,&settings);BrowserAuthManager auth(&settings,&web);AudioTransport audio;
        QFile source(wav(dir.filePath("source.wav")));QVERIFY(source.open(QIODevice::ReadOnly));audio.payload=source.readAll();
        DownloadManager downloads(&settings,&importer,nullptr,&auth,nullptr,&audio);
        LibraryExchange exchange(&library,&settings,nullptr,nullptr,&web);
        connect(&exchange,&LibraryExchange::audioImportRequested,&downloads,[&](const QVariantList &tracks){downloads.enqueueNative(tracks,"original");});
        QSignalSpy loggedIn(&web,&WebLoginManager::loginDone),completed(&exchange,&LibraryExchange::completed),automatic(&exchange,&LibraryExchange::audioImportRequested);
        exchange.importMyMusic("VK",profile);QTRY_VERIFY_WITH_TIMEOUT(!completed.isEmpty(),14000);
        QCOMPARE(automatic.count(),1);QTRY_VERIFY_WITH_TIMEOUT(!downloads.busy(),6000);
        QVERIFY(loggedIn.isEmpty());QVERIFY(!web.browserVisible());QVERIFY(web.nativeProfile()==nullptr);QVERIFY(session);
        const auto *process=session->findChild<QProcess*>();QVERIFY(process);
        QVERIFY(process->arguments().contains("--headless=new"));QVERIFY(!process->arguments().contains("--app=about:blank"));
        for(const auto &argument:process->arguments())QVERIFY(!argument.contains(profile));
        bool cleared=false,seeded=false;
        for(const auto &call:protocol.commands){if(call.first=="Network.clearBrowserCookies")cleared=true;
            if(call.first=="Network.setCookie" && call.second.value("name").toString()=="remixsid6" && call.second.value("value").toString()=="fixture-session")seeded=true;}
        QVERIFY(cleared);QVERIFY(seeded);QCOMPARE(library.repository().count(),1);
        const auto track=library.repository().tracks().first();QVERIFY(QFileInfo::exists(track.filePath));
        QCOMPARE(downloads.data(downloads.index(0),DownloadManager::StatusRole).toString(),QString("succeeded"));web.logout("vk");
#endif
    }
    void relationalLibraryAndCascade() {
        QTemporaryDir dir;LibraryRepository repo(dir.filePath("library.db"));QVERIFY2(repo.ready(),qPrintable(repo.error()));
        Track t;t.title="Ночь ' OR 1=1 --";t.artist="МЫ";t.album="Город";t.source="VK";
        int id=repo.saveTrack(t);QVERIFY(id>0);QCOMPARE(repo.saveTrack(t),id);
        Track other=t;other.title="Утро";int id2=repo.saveTrack(other);QVERIFY(id2!=id);
        int list=repo.createPlaylist("Вечер");QVERIFY(repo.addToPlaylist(list,id2));QVERIFY(repo.addToPlaylist(list,id));QVERIFY(repo.addToPlaylist(list,id));
        auto tracks=repo.tracks({},list);QCOMPARE(tracks.size(),2);QCOMPARE(tracks[0].id,id2);QCOMPARE(tracks[1].id,id);
        QCOMPARE(repo.tracks("ночь").size(),1);QCOMPARE(repo.tracks("мы").size(),2);
        QVERIFY(repo.setRating(id,100));QCOMPARE(repo.track(id).rating,5);QCOMPARE(repo.tracks({},-1).size(),1);
        QVERIFY(repo.recordListen(id,4200,false));QCOMPARE(repo.track(id).listenMs,4200);
        QVERIFY(!repo.recordListen(9999,100,false));QCOMPARE(repo.track(id).listenMs,4200);
        QVERIFY(repo.removeTrack(id));QCOMPARE(repo.tracks({},list).size(),1);QVERIFY(repo.deletePlaylist(list));QCOMPARE(repo.tracks().size(),1);
    }
    void legacyMigrationPreservesIdsAndStatistics() {
        QTemporaryDir dir;QString path=dir.filePath("legacy.db");
        {
            auto db=QSqlDatabase::addDatabase("QSQLITE","legacy_fixture");db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);
            QVERIFY(q.exec("CREATE TABLE tracks(id INTEGER PRIMARY KEY,title TEXT NOT NULL,artist TEXT NOT NULL,album TEXT NOT NULL,file_path TEXT NOT NULL UNIQUE,source_url TEXT NOT NULL DEFAULT '',source TEXT NOT NULL,format TEXT NOT NULL,duration INTEGER NOT NULL,bitrate INTEGER NOT NULL,rating INTEGER NOT NULL,play_count INTEGER NOT NULL)"));
            QVERIFY(q.exec("INSERT INTO tracks VALUES(42,'Песня','Артист','Альбом','/some/song.mp3','','Local','MP3',120000,320,5,9)"));
        }QSqlDatabase::removeDatabase("legacy_fixture");
        {LibraryRepository repo(path);QVERIFY2(repo.ready(),qPrintable(repo.error()));auto t=repo.track(42);QCOMPARE(t.title,QString("Песня"));QCOMPARE(t.rating,5);QCOMPARE(t.playCount,9);QCOMPARE(t.duration,120000);QCOMPARE(t.artist,QString("Артист"));}
        LibraryRepository reopened(path);QVERIFY(reopened.ready());QCOMPARE(reopened.tracks().size(),1);
    }
    void failedMigrationRollsBack() {
        QTemporaryDir dir;QString path=dir.filePath("bad.db");
        {auto db=QSqlDatabase::addDatabase("QSQLITE","bad_fixture");db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);QVERIFY(q.exec("CREATE TABLE tracks(id INTEGER PRIMARY KEY,title TEXT)"));QVERIFY(q.exec("INSERT INTO tracks VALUES(7,'Preserve me')"));}
        QSqlDatabase::removeDatabase("bad_fixture");
        {LibraryRepository repo(path);QVERIFY(!repo.ready());}
        {auto db=QSqlDatabase::addDatabase("QSQLITE","check_fixture");db.setDatabaseName(path);QVERIFY(db.open());QSqlQuery q(db);QVERIFY(q.exec("SELECT title FROM tracks WHERE id=7"));QVERIFY(q.next());QCOMPARE(q.value(0).toString(),QString("Preserve me"));QVERIFY(!db.tables().contains("tracks_v1"));}
        QSqlDatabase::removeDatabase("check_fixture");
    }
    void parsersPreserveNamesAndRejectMalformedInput() {
        QString error;
        auto txt=LibraryExchange::parseText(QString("\ufeffМЫ — Ночь\nAC-DC - Back - Live\nОдна песня\n").toUtf8(),"txt","VK",&error);
        QVERIFY(error.isEmpty());QCOMPARE(txt.size(),3);QCOMPARE(txt[1].title,QString("Back - Live"));QCOMPARE(txt[0].artist,QString("МЫ"));
        auto csv=LibraryExchange::parseText("artist,title,album\r\n\"A, B\",\"Song \"\"one\"\"\nLive\",Record\r\n","csv","VK",&error);
        QCOMPARE(csv.size(),1);QCOMPARE(csv[0].artist,QString("A, B"));QCOMPARE(csv[0].title,QString("Song \"one\"\nLive"));
        auto invalid=LibraryExchange::parseText("artist,title\nA,\"unfinished","csv","VK",&error);QVERIFY(invalid.isEmpty());QVERIFY(!error.isEmpty());
        error.clear();auto json=LibraryExchange::parseText("{\"result\":{\"tracks\":[{\"track\":{\"id\":123,\"title\":\"Test\",\"artists\":[{\"name\":\"Singer\"}],\"durationMs\":1234}}]}}","json","Яндекс Музыка",&error);
        QCOMPARE(json.size(),1);QCOMPARE(json[0].sourceUrl,QString("https://music.yandex.ru/track/123"));QCOMPARE(json[0].duration,1234);
    }
    void localImportDuplicateAndFilenameCollision() {
        QTemporaryDir dir;QDir(dir.path()).mkpath("a");QDir(dir.path()).mkpath("b");
        auto first=wav(dir.filePath("a/track.wav"),1),second=wav(dir.filePath("b/track.wav"),2);
        SettingsStore settings;settings.setLibraryFolder(dir.filePath("music"));LibraryModel library(nullptr,dir.filePath("library.db"));ImportManager importer(&library,&settings);
        QSignalSpy finished(&importer,&ImportManager::importCompleted);
        importer.importPaths({first,second,first});QTRY_COMPARE_WITH_TIMEOUT(finished.size(),1,20000);
        QCOMPARE(library.totalCount(),2);QCOMPARE(finished.first()[0].toInt(),2);QCOMPARE(finished.first()[1].toInt(),1);
        QVERIFY(QFileInfo::exists(dir.filePath("music/track.wav")));QVERIFY(QFileInfo::exists(dir.filePath("music/track (1).wav")));
        QFile a(first),b(second);QVERIFY(a.open(QIODevice::ReadOnly));QVERIFY(b.open(QIODevice::ReadOnly));QVERIFY(a.readAll()!=b.readAll());
    }
    void queueSurvivesFilterAndTracksRealTime() {
        QTemporaryDir dir;SettingsStore settings;settings.setLibraryFolder(dir.filePath("music"));settings.setVolume(0);
        LibraryModel library(nullptr,dir.filePath("library.db"));
        QVERIFY(library.addTrack(wav(dir.filePath("A.wav"))));QVERIFY(library.addTrack(wav(dir.filePath("B.wav"))));
        PlaybackController player(&library,&settings);player.setShuffle(false);player.setRepeatMode(0);
        player.playIndex(0);int id=player.currentTrackId();QVERIFY(id>0);QCOMPARE(player.queue().size(),2);
        library.setSearchText("no match");QCOMPARE(library.count(),0);QCOMPARE(player.currentTrackId(),id);
        player.next();QVERIFY(player.currentTrackId()!=id);QCOMPARE(player.queue().size(),2);
        QTRY_VERIFY_WITH_TIMEOUT(player.duration()>0,5000);
        player.seek(9000);QTest::qWait(150);player.stop();
        QVERIFY(library.trackById(player.currentTrackId()).listenMs<4000);
        player.previous();QCOMPARE(player.currentTrackId(),id);
    }
    void decodedPlaybackDrivesAtmosphereAndResetsOnPause() {
        QTemporaryDir dir;const auto path=dir.filePath("rhythm.wav");QFile file(path);QVERIFY(file.open(QIODevice::WriteOnly));
        QDataStream stream(&file);stream.setByteOrder(QDataStream::LittleEndian);
        constexpr int rate=48000,frames=rate*4,bytes=frames*2;
        stream.writeRawData("RIFF",4);stream<<quint32(36+bytes);stream.writeRawData("WAVEfmt ",8);
        stream<<quint32(16)<<quint16(1)<<quint16(1)<<quint32(rate)<<quint32(rate*2)<<quint16(2)<<quint16(16);
        stream.writeRawData("data",4);stream<<quint32(bytes);
        for(int frame=0;frame<frames;++frame) {
            const double time=double(frame)/rate,beat=std::fmod(time,.5);
            stream<<qint16(beat<.08?22000*std::exp(-beat*25)*std::sin(time*2*3.141592653589793*65):0);
        }file.close();
        SettingsStore settings;const bool previousMotion=settings.motionEnabled();
        const auto restoreMotion=qScopeGuard([&]{settings.setMotionEnabled(previousMotion);});
        settings.setMotionEnabled(true);settings.setVolume(0);
        LibraryModel library(nullptr,dir.filePath("library.db"));QVERIFY(library.addTrack(path));
        PlaybackController player(&library,&settings);double peakPulse=0,peakBass=0;
        connect(&player,&PlaybackController::audioReactivityChanged,this,[&]{peakPulse=qMax(peakPulse,player.beatPulse());peakBass=qMax(peakBass,player.bassEnergy());});
        player.playIndex(0);QTRY_VERIFY_WITH_TIMEOUT(peakPulse>.2 && peakBass>.2,3500);
        QVERIFY(player.audioLevels().size()==24);player.pause();QVERIFY(!player.playing());
        QCOMPARE(player.beatPulse(),0.0);QCOMPARE(player.audioEnergy(),0.0);QCOMPARE(player.bassEnergy(),0.0);
        player.play();QTRY_VERIFY_WITH_TIMEOUT(player.audioEnergy()>.1,1500);
        settings.setMotionEnabled(false);QCOMPARE(player.beatPulse(),0.0);QCOMPARE(player.audioEnergy(),0.0);
        player.stop();
    }
    void youtubeSearchUsesArgumentsAndRequiresSelection() {
#ifdef Q_OS_WIN
        QSKIP("This process fixture uses a POSIX shell");
#else
        QTemporaryDir dir;const QString executable=dir.filePath("fake-yt-dlp"),arguments=dir.filePath("arguments.txt");
        QFile script(executable);QVERIFY(script.open(QIODevice::WriteOnly));
        script.write("#!/bin/sh\nif [ \"$1\" = \"--version\" ]; then echo 2026.10.26; exit 0; fi\nprintf '%s\\n' \"$@\" > '"+arguments.toUtf8()+"'\n");
        script.write("cat <<'JSON'\n{\"entries\":[{\"id\":\"abc123\",\"title\":\"Test song\",\"uploader\":\"Artist\",\"duration\":120,\"url\":\"https://www.youtube.com/watch?v=abc123\"},{\"title\":\"Ignore unsupported source\",\"url\":\"https://example.invalid/media\"}]}\nJSON\n");script.close();QVERIFY(script.setPermissions(QFileDevice::ReadOwner|QFileDevice::WriteOwner|QFileDevice::ExeOwner));
        SettingsStore settings;settings.setYtDlpPath(executable);settings.setLibraryFolder(dir.filePath("music"));
        LibraryModel library(nullptr,dir.filePath("db"));ImportManager importer(&library,&settings);DependencyManager dependencies(&settings);BrowserAuthManager auth(&settings,nullptr);DownloadManager downloads(&settings,&importer,&dependencies,&auth);ImportPreviewer preview(&settings,&dependencies,&auth,&downloads);
        const QString query="Artist; printf NOT_A_COMMAND";preview.searchYoutube(query,"balanced");QTRY_VERIFY_WITH_TIMEOUT(!preview.busy(),5000);
        QVERIFY2(preview.lastError().isEmpty(),qPrintable(preview.lastError()));QCOMPARE(preview.rowCount(),1);QCOMPARE(preview.selectedCount(),0);
        QFile args(arguments);QVERIFY(args.open(QIODevice::ReadOnly));const auto text=args.readAll();QVERIFY(text.contains(("ytsearch10:"+query).toUtf8()));QVERIFY(text.contains("--ignore-config"));
        preview.toggle(0);QCOMPARE(preview.selectedCount(),1);
        preview.inspect("file:///etc/passwd","balanced");QVERIFY(!preview.lastError().isEmpty());
#endif
    }
    void sourceAttachmentKeepsPlaylistIdentity() {
        QTemporaryDir dir;LibraryRepository repo(dir.filePath("library.db"));
        Track metadata;metadata.title="Song";metadata.artist="Singer";metadata.source="YouTube";metadata.sourceUrl="https://www.youtube.com/watch?v=abc";
        int id=repo.saveTrack(metadata),playlist=repo.createPlaylist("Keep order");QVERIFY(repo.addToPlaylist(playlist,id));QVERIFY(repo.setRating(id,5));
        Track downloaded;downloaded.title="Singer - Song (Official Audio)";downloaded.artist="Channel";downloaded.source="YouTube";downloaded.sourceUrl=metadata.sourceUrl;downloaded.filePath=wav(dir.filePath("download.wav"));downloaded.format="WAV";
        QCOMPARE(repo.saveTrack(downloaded),id);QCOMPARE(repo.count(),1);QCOMPARE(repo.track(id).title,metadata.title);QCOMPARE(repo.track(id).rating,5);QCOMPARE(repo.tracks({},playlist).first().filePath,downloaded.filePath);
    }
    void invalidAudioIsNotAdded() {
        if (QStandardPaths::findExecutable("ffprobe").isEmpty()) QSKIP("ffprobe required for invalid audio validation");
        QTemporaryDir dir;QString path=dir.filePath("broken.mp3");QFile f(path);QVERIFY(f.open(QIODevice::WriteOnly));f.write("This is not audio");f.close();
        LibraryModel library(nullptr,dir.filePath("library.db"));QVERIFY(!library.addTrack(path));QCOMPARE(library.count(),0);QVERIFY(!library.lastError().isEmpty());
    }
    void namelessThemeUpdatesWithoutRestart() {
        const QString config=QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)+"/hypr-themes";
        const QString state=QStandardPaths::writableLocation(QStandardPaths::GenericStateLocation)+"/hypr-theme";
        QDir().mkpath(config+"/themes");QDir().mkpath(config+"/palettes");QDir().mkpath(state);
        auto write=[](const QString &path,const QByteArray &data){QFile f(path);if(!f.open(QIODevice::WriteOnly|QIODevice::Truncate))return false;return f.write(data)==data.size();};
        QVERIFY(write(config+"/palettes/test.json","{\"colors\":{\"blue\":\"#123456\",\"lavender\":\"#654321\"}}"));
        QVERIFY(write(config+"/themes/test.json","{\"name\":\"Testing\",\"palette\":\"test\"}"));QVERIFY(write(state+"/current","test\n"));
        NamelessBridge bridge;QVERIFY(bridge.connected());QCOMPARE(bridge.accent(),QColor("#123456"));QCOMPARE(bridge.themeName(),QString("Testing"));
        QVERIFY(write(config+"/themes/test.json","{\"name\":\"Changed\",\"palette\":\"test\",\"accent\":\"#abcdef\"}"));
        QTRY_COMPARE_WITH_TIMEOUT(bridge.accent(),QColor("#abcdef"),3000);QCOMPARE(bridge.themeName(),QString("Changed"));
    }
    void portableRoundTripAndPathTraversal() {
        QTemporaryDir dir;SettingsStore settings;settings.setLibraryFolder(dir.filePath("source-music"));
        LibraryModel library(nullptr,dir.filePath("library.db"));QVERIFY(library.addTrack(wav(dir.filePath("song.wav"))));
        int track=library.repository().tracks()[0].id;library.setRating(track,4);int playlist=library.createPlaylist("Travel");library.addToPlaylist(playlist,track);
        Track metadata;metadata.title="Without audio";metadata.artist="Singer";metadata.source="VK";int missing=library.repository().saveTrack(metadata);library.addToPlaylist(playlist,missing);
        LibraryExchange exchange(&library,&settings,nullptr,nullptr,nullptr);QDir(dir.path()).mkpath("export");exchange.exportPortable(QUrl::fromLocalFile(dir.filePath("export")),0);
        QTRY_VERIFY_WITH_TIMEOUT(!exchange.busy(),10000);
        auto names=QDir(dir.filePath("export")).entryList(QDir::Dirs|QDir::NoDotAndDotDot);QCOMPARE(names.size(),1);
        QString bundle=dir.filePath("export/"+names.first()+"/aurora-library.json");QVERIFY(QFileInfo::exists(bundle));
        settings.setLibraryFolder(dir.filePath("restored"));LibraryModel restored(nullptr,dir.filePath("restored.db"));LibraryExchange restore(&restored,&settings,nullptr,nullptr,nullptr);
        restore.importList(QUrl::fromLocalFile(bundle),"Aurora");QCOMPARE(restored.totalCount(),2);QCOMPARE(restored.playlists().size(),1);
        auto list=restored.repository().tracks({},restored.playlists().first().toMap()["id"].toInt());QCOMPARE(list.size(),2);QCOMPARE(list[0].rating,4);QVERIFY(list[0].filePath.startsWith(dir.filePath("restored/")));QVERIFY(QFileInfo::exists(list[0].filePath));QVERIFY(list[1].filePath.isEmpty());
        QFile f(bundle);QVERIFY(f.open(QIODevice::ReadOnly));auto doc=QJsonDocument::fromJson(f.readAll()).object();f.close();auto tracks=doc["tracks"].toArray();auto bad=tracks[0].toObject();bad["file"]="../../../song.wav";tracks[0]=bad;doc["tracks"]=tracks;
        QString malicious=dir.filePath("bad.json");QFile out(malicious);QVERIFY(out.open(QIODevice::WriteOnly));out.write(QJsonDocument(doc).toJson());out.close();
        int before=restored.totalCount();restore.importList(QUrl::fromLocalFile(malicious),"Aurora");QCOMPARE(restored.totalCount(),before);QVERIFY(restore.status().contains("путь"));
    }
    void downloaderErrorsAreHumanized() {
        using DownloaderErrors::humanizeDownloaderError;
        QCOMPARE(humanizeDownloaderError(
            "ERROR: could not find firefox cookies database in /home/u/.mozilla/firefox", QStringLiteral("firefox")),
            QString::fromUtf8("Не удалось прочитать cookies браузера «firefox». Войдите на сайт в этом браузере "
                              "и закройте все его окна, либо выберите другой браузер в настройках."));
        QCOMPARE(humanizeDownloaderError("ERROR: failed to load cookies"),
                 QString::fromUtf8("Не удалось прочитать cookies браузера. Войдите на сайт в этом браузере "
                                   "и закройте все его окна, либо выберите другой браузер в настройках."));
        QCOMPARE(humanizeDownloaderError("ERROR: Sign in to confirm you're not a bot"),
                 QString::fromUtf8("YouTube запросил проверку «не робот». Подождите минуту и повторите, "
                                   "либо включите авторизованный режим с разрешённым браузером."));
        QCOMPARE(humanizeDownloaderError("ERROR: Video unavailable"), QString::fromUtf8("Видео недоступно (удалено или приватное)."));
        QCOMPARE(humanizeDownloaderError("ERROR: Private video"), QString::fromUtf8("Видео недоступно (удалено или приватное)."));
        QCOMPARE(humanizeDownloaderError("ERROR: unable to extract player response; please report this issue"),
                 QString::fromUtf8("Ссылка не распознана загрузчиком. Проверьте адрес."));
        QCOMPARE(humanizeDownloaderError("ERROR: Unsupported URL: https://example.invalid/page"),
                 QString::fromUtf8("Ссылка не распознана загрузчиком. Проверьте адрес."));
        QCOMPARE(humanizeDownloaderError("ERROR: HTTP Error 403: Forbidden"),
                 QString::fromUtf8("Сервис отказал в доступе (403). Для личных плейлистов используйте авторизованный режим."));
        QCOMPARE(humanizeDownloaderError("что-то неизвестное\nпоследняя строка ошибки"), QStringLiteral("последняя строка ошибки"));
        QVERIFY(humanizeDownloaderError(QString()).isEmpty());
    }
    void browserDetectionScansHomeAndReadsFirefoxProfile() {
        QTemporaryDir dir;
        QVERIFY(QDir().mkpath(dir.filePath(".config/BraveSoftware/Brave-Browser")));
        QFile localState(dir.filePath(".config/BraveSoftware/Brave-Browser/Local State"));
        QVERIFY(localState.open(QIODevice::WriteOnly));localState.write("{}");localState.close();
        QVERIFY(QDir().mkpath(dir.filePath(".mozilla/firefox")));
        QFile profiles(dir.filePath(".mozilla/firefox/profiles.ini"));
        QVERIFY(profiles.open(QIODevice::WriteOnly));
        profiles.write("[Profile0]\nName=default\nIsRelative=1\nPath=abc.default\nDefault=1\n\n[General]\nStartWithLastProfile=1\n");
        profiles.close();
        const auto found = BrowserAuthManager::detectBrowsers(dir.path());
        QCOMPARE(found, QStringList({"firefox", "brave"}));
        QVERIFY(!found.contains(QStringLiteral("yandexbrowser")));
        QCOMPARE(BrowserAuthManager::detectFirefoxProfile(dir.path()), QStringLiteral("default"));
        QTemporaryDir empty;
        QVERIFY(BrowserAuthManager::detectBrowsers(empty.path()).isEmpty());
        QCOMPARE(BrowserAuthManager::detectFirefoxProfile(empty.path()), QString());
    }
    void oauthAuthorizeUrlsAndRedirectParsing() {
        const QString vk=OAuthManager::vkAuthorizeUrl("12345");
        QVERIFY(vk.contains("client_id=12345"));
        QVERIFY(vk.contains("redirect_uri=http%3A%2F%2F127.0.0.1%3A18765%2F"));
        QVERIFY(vk.contains("response_type=code"));QVERIFY(vk.contains("scope=audio"));QVERIFY(vk.contains("state=aurora"));
        const QString ya=OAuthManager::yandexAuthorizeUrl("abc678");
        QVERIFY(ya.contains("client_id=abc678"));
        QVERIFY(ya.contains("response_type=code"));
        QVERIFY(ya.contains("redirect_uri=http%3A%2F%2F127.0.0.1%3A18765%2F"));
        QString code,token,state,error;qint64 expires=0;
        QVERIFY(OAuthManager::parseRedirectQuery("/?code=abc123&state=aurora",&code,&token,&expires,&state,&error));
        QCOMPARE(code,QString("abc123"));QCOMPARE(state,QString("aurora"));QVERIFY(token.isEmpty());QVERIFY(error.isEmpty());
        code.clear();token.clear();state.clear();
        QVERIFY(OAuthManager::parseRedirectQuery("/#access_token=tok987&expires_in=3600",&code,&token,&expires,&state,&error));
        QCOMPARE(token,QString("tok987"));QCOMPARE(expires,qint64(3600));
        QVERIFY(!OAuthManager::parseRedirectQuery("/?error=access_denied&error_description=no",&code,&token,&expires,&state,&error));
        QVERIFY(error.contains("access_denied"));
        QVERIFY(!OAuthManager::parseRedirectQuery("/just/a/path",&code,&token,&expires,&state,&error));
    }
    void oauthTokenJsonAppliesToSettings() {
        SettingsStore settings;QString message;
        QVERIFY(OAuthManager::applyTokenJson(&settings,"VK","{\"access_token\":\"vktok\",\"expires_in\":0,\"user_id\":1}",&message));
        QCOMPARE(settings.vkToken(),QString("vktok"));QCOMPARE(message,QString("VK подключён."));
        const qint64 before=QDateTime::currentSecsSinceEpoch();
        QVERIFY(OAuthManager::applyTokenJson(&settings,"Яндекс Музыка","{\"access_token\":\"yatok\",\"refresh_token\":\"refresh1\",\"expires_in\":3600}",&message));
        QCOMPARE(settings.yandexToken(),QString("yatok"));QCOMPARE(settings.yandexRefreshToken(),QString("refresh1"));
        QVERIFY(settings.yandexTokenExpiry()>=before+3600);
        QVERIFY(!OAuthManager::applyTokenJson(&settings,"VK","{\"error\":\"invalid_grant\",\"error_description\":\"bad code\"}",&message));
        QVERIFY(message.contains("invalid_grant"));QCOMPARE(settings.vkToken(),QString("vktok"));
        QVERIFY(!OAuthManager::applyTokenJson(&settings,"VK","not json",&message));
    }
    void vkAudioParsingAndApiErrors() {
        QString error,title;
        auto tracks=VkProvider::parseAudioGet("{\"response\":{\"count\":2,\"title\":\"Вечер\",\"items\":[\n            {\"id\":10,\"owner_id\":99,\"artist\":\"МЫ\",\"title\":\"Ночь\",\"duration\":180},\n            {\"id\":11,\"owner_id\":99,\"artist\":\"AC-DC\",\"title\":\"Back\",\"duration\":240}]}}",&error,&title);
        QVERIFY(error.isEmpty());QCOMPARE(tracks.size(),2);QCOMPARE(title,QString("Вечер"));
        QCOMPARE(tracks[0].artist,QString("МЫ"));QCOMPARE(tracks[0].title,QString("Ночь"));
        QCOMPARE(tracks[0].duration,qint64(180000));QCOMPARE(tracks[0].sourceUrl,QString("https://vk.com/audio99_10"));
        QCOMPARE(tracks[0].source,QString("VK"));QCOMPARE(tracks[1].duration,qint64(240000));
        VkProvider::parseAudioGet("{\"error\":{\"error_code\":5,\"error_msg\":\"User authorization failed\"}}",&error,&title);
        QVERIFY(error.contains("недействителен"));
        VkProvider::parseAudioGet("{\"error\":{\"error_code\":201,\"error_msg\":\"Access to audio denied\"}}",&error,&title);
        QVERIFY(error.contains("доступ"));QVERIFY(error.contains("верифицированн"));
        VkProvider::parseAudioGet("{\"error\":{\"error_code\":14,\"error_msg\":\"Captcha needed\"}}",&error,&title);
        QVERIFY(error.contains("Captcha"));
        VkProvider::parseAudioGet("garbage",&error,&title);
        QVERIFY(!error.isEmpty());
    }
    void cdpFramePackingAndParsing() {
        const QByteArray payload="hi";
        const quint32 mask=0x01020304;
        const auto frame=CdpClient::packFrame(payload,mask);
        QCOMPARE(frame.size(),2+4+2);
        QCOMPARE(quint8(frame.at(0)),quint8(0x81));
        QCOMPARE(quint8(frame.at(1)),quint8(0x82));
        QCOMPARE(quint8(frame.at(2)),quint8(0x01));QCOMPARE(quint8(frame.at(3)),quint8(0x02));
        QCOMPARE(quint8(frame.at(4)),quint8(0x03));QCOMPARE(quint8(frame.at(5)),quint8(0x04));
        QByteArray buffer=frame;
        const auto parsed=CdpClient::feedData(&buffer);
        QCOMPARE(parsed.size(),1);QCOMPARE(parsed[0].opcode,quint8(1));QCOMPARE(parsed[0].payload,payload);QVERIFY(buffer.isEmpty());
        const QByteArray big(200,'x');
        const auto f126=CdpClient::packFrame(big,1);
        QCOMPARE(quint8(f126.at(1)),quint8(0x80|126));
        QCOMPARE((quint16(quint8(f126.at(2)))<<8)|quint8(f126.at(3)),quint16(200));
        const QByteArray huge(70000,'y');
        const auto f127=CdpClient::packFrame(huge,1);
        QCOMPARE(quint8(f127.at(1)),quint8(0x80|127));
        QCOMPARE(f127.size(),10+4+70000);
        QByteArray fragmented=frame.left(4);
        QVERIFY(CdpClient::feedData(&fragmented).isEmpty());
        QCOMPARE(fragmented.size(),4);
        fragmented+=frame.mid(4);
        const auto rest=CdpClient::feedData(&fragmented);
        QCOMPARE(rest.size(),1);QCOMPARE(rest[0].payload,payload);
        QByteArray coalesced=frame+frame;
        const auto two=CdpClient::feedData(&coalesced);
        QCOMPARE(two.size(),2);QVERIFY(coalesced.isEmpty());
        QByteArray ping;ping.append(char(0x89));ping.append(char(0));
        const auto pings=CdpClient::feedData(&ping);
        QCOMPARE(pings.size(),1);QCOMPARE(pings[0].opcode,quint8(9));
    }
    void webLoginCookieHelpersAndNetscapeExport() {
        QCOMPARE(WebLoginManager::findExecutableIn("sh",{"/bin","/nonexistent"}),QString("/bin/sh"));
        QVERIFY(WebLoginManager::findExecutableIn("definitely-missing-aurora",{}).isEmpty());
        WebLoginManager::StoredCookie session{"remixsid","secret",".vk.com","/",0,true,true};
        WebLoginManager::StoredCookie other{"other","x",".vk.com","/",0,false,false};
        QVERIFY(WebLoginManager::hasSessionCookie({session,other},"vk"));
        QVERIFY(!WebLoginManager::hasSessionCookie({other},"vk"));
        QVERIFY(!WebLoginManager::hasSessionCookie({session},"yandex"));
        WebLoginManager::StoredCookie expired{"Session_id","s",".yandex.ru","/",QDateTime::currentSecsSinceEpoch()-10,false,false};
        QVERIFY(!WebLoginManager::hasSessionCookie({expired},"yandex"));
        QCOMPARE(WebLoginManager::normalizeService("Яндекс Музыка"),QString("yandex"));
        QCOMPARE(WebLoginManager::normalizeService("VK"),QString("vk"));
        WebLoginManager manager(nullptr);
        manager.saveCookies("vk",{session,other});
        QVERIFY(manager.hasSession("vk"));
        QVERIFY(!manager.hasSession("yandex"));
        const QString header=manager.cookieHeader("vk",QUrl("https://vk.com/audios1"));
        QVERIFY(header.contains("remixsid=secret"));QVERIFY(header.contains("other=x"));
        const QString plain=manager.cookieHeader("vk",QUrl("http://vk.com/"));
        QCOMPARE(plain,QString("other=x"));
        const QString path=manager.exportNetscape("vk");
        QVERIFY(!path.isEmpty());
        QFile file(path);QVERIFY(file.open(QIODevice::ReadOnly));
        const auto text=QString::fromUtf8(file.readAll());file.close();
        QVERIFY(text.contains("# Netscape HTTP Cookie File"));
        QVERIFY(text.contains("#HttpOnly_.vk.com\tTRUE\t/\tTRUE\t0\tremixsid\tsecret"));
        QVERIFY(text.contains(".vk.com\tTRUE\t/\tFALSE\t0\tother\tx"));
        QFile::remove(path);
        manager.logout("vk");QVERIFY(!manager.hasSession("vk"));
    }
    void vkWebParsingDataAudioAndLoadSection() {
        const QString html=QStringLiteral(
            "<div id=\"initial_player\" data-audio=\"[[456239066,37115248,&quot;&quot;,&quot;Ночь&quot;,&quot;МЫ&quot;,180],"
            "[456239067,37115249,&quot;&quot;,&quot;Back&quot;,&quot;AC-DC&quot;,240]]\"></div>");
        const auto tracks=VkProvider::parseDataAudio(html);
        QCOMPARE(tracks.size(),2);
        QCOMPARE(tracks[0].artist,QString("МЫ"));QCOMPARE(tracks[0].title,QString("Ночь"));
        QCOMPARE(tracks[0].duration,qint64(180000));
        QCOMPARE(tracks[0].sourceUrl,QString("https://vk.com/audio37115248_456239066"));
        QCOMPARE(tracks[1].duration,qint64(240000));
        QVERIFY(VkProvider::parseDataAudio("<html>no data here</html>").isEmpty());
        const auto flat=VkProvider::parseDataAudio("<div data-audio='[10,99,\"\",\"First\",\"Artist\",180]'></div><div data-audio='[11,99,\"\",\"Second\",\"Artist\",240]'></div>");
        QCOMPARE(flat.size(),2);QCOMPARE(flat[0].sourceUrl,QString("https://vk.com/audio99_10"));
        const QByteArray section="{\"payload\":[1,[[10,[456239066,37115248,\"\",\"Ночь\",\"МЫ\",180],[456239067,37115249,\"\",\"Back\",\"AC-DC\",240]],\"Вечерний плейлист\"]]}";
        QString name,error;qint64 total=0;
        const auto loaded=VkProvider::parseLoadSection(section,&name,&error,&total);
        QVERIFY2(error.isEmpty(),qPrintable(error));
        QCOMPARE(total,qint64(10));QCOMPARE(name,QString("Вечерний плейлист"));
        QCOMPARE(loaded.size(),2);QCOMPARE(loaded[1].title,QString("Back"));
    }

    void cdpHandshakeUsesUpgradeAndValidatesServer() {
        const QByteArray key="dGhlIHNhbXBsZSBub25jZQ==";
        const auto request=CdpClient::handshakeRequest(QUrl("ws://127.0.0.1:9222/devtools/page/42?x=1"),key);
        QVERIFY(request.startsWith("GET /devtools/page/42?x=1 HTTP/1.1\r\n"));
        QVERIFY(request.contains("Host: 127.0.0.1:9222\r\n"));
        QVERIFY(request.contains("Upgrade: websocket\r\nConnection: Upgrade\r\n"));
        QVERIFY(request.contains("Sec-WebSocket-Key: "+key+"\r\n"));
        QVERIFY(request.endsWith("Sec-WebSocket-Version: 13\r\n\r\n"));
        const QByteArray response="HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n";
        QVERIFY(CdpClient::acceptsHandshake(response,key));
        QVERIFY(!CdpClient::acceptsHandshake(response,"wrong"));
        QVERIFY(!CdpClient::acceptsHandshake("HTTP/1.1 200 OK\r\n",key));
        CdpClient client;QSignalSpy failed(&client,&CdpClient::connectFailed);
        client.connectToHost(QUrl("ws://example.com:9222/"));QCOMPARE(failed.count(),1);
    }
    void nativeHarvestChecksOriginsAndSessionScope() {
        QCOMPARE(WebLoginManager::serviceForHost("login.vk.ru"),QString("vk"));
        QCOMPARE(WebLoginManager::serviceForHost("music.yandex.kz"),QString("yandex"));
        QVERIFY(WebLoginManager::serviceForHost("vk.ru.example.com").isEmpty());
        QVERIFY(WebLoginManager::serviceForHost("notyandex.ru").isEmpty());
        const QByteArray fixture="{\"tracks\":[\n            {\"url\":\"https://vk.ru/audio99_10\",\"title\":\"Ночь\",\"artist\":\"МЫ\",\"durationMs\":180000,\"mediaUrl\":\"https://cdn.example/audio.mp3\",\"rawAudio\":[10,99]},\n            {\"url\":\"https://vk.ru/audio99_10\",\"title\":\"Дубликат\"},\n            {\"url\":\"https://vk.ru.evil.test/audio99_10\",\"title\":\"Чужой домен\"},\n            {\"url\":\"http://vk.ru/audio99_11\",\"title\":\"HTTP\"},\n            {\"url\":\"https://music.yandex.ru/track/12\",\"title\":\"Другой сервис\"}]}";
        const auto tracks=WebLoginManager::parseHarvest(fixture,"vk");QCOMPARE(tracks.size(),1);
        QCOMPARE(tracks[0].duration,qint64(180000));QCOMPARE(tracks[0].title,QString("Ночь"));
        QCOMPARE(tracks[0].mediaUrl,QString("https://cdn.example/audio.mp3"));
        QCOMPARE(tracks[0].webData.value("rawAudio").toList().size(),2);
        QVERIFY(!MediaSniffer::isAudioUrl(QUrl("http://cdn.example/file.mp3")));
        QVERIFY(!MediaSniffer::isAudioUrl(QUrl("https://name:password@cdn.example/file.mp3")));
        QVERIFY(!MediaSniffer::isAudioUrl(QUrl("https://cdn.example/file.mp3.exe")));
        QVERIFY(MediaSniffer::isAudioUrl(QUrl("https://cdn.example/get-mp3/hash/track?secret=fixture")));
        WebLoginManager manager(nullptr);
        manager.saveCookies("vk",{{"remixsid","value",".vk.ru","/audio",0,true,true}});
        QVERIFY(manager.cookieHeader("vk",QUrl("https://vk.ru/audios1")).isEmpty());
        QVERIFY(manager.cookieHeader("vk",QUrl("https://vk.ru.evil.test/audio/1")).isEmpty());
        QVERIFY(manager.cookieHeader("vk",QUrl("http://vk.ru/audio/1")).isEmpty());
        QVERIFY(!manager.cookieHeader("vk",QUrl("https://vk.ru/audio/1")).isEmpty());
        const QString path=manager.exportNetscape("vk");
        const auto permissions=QFile::permissions(path);
        QVERIFY(!(permissions & (QFile::ReadGroup|QFile::WriteGroup|QFile::ReadOther|QFile::WriteOther)));
        manager.logout("vk");
    }
    void nativeDownloadRejectsEncryptedPacketsWithValidContainer() {
        const auto ffmpeg=QStandardPaths::findExecutable("ffmpeg");
        if(ffmpeg.isEmpty() || QStandardPaths::findExecutable("ffprobe").isEmpty())QSKIP("FFmpeg required");
        QTemporaryDir dir;const auto source=wav(dir.filePath("source.wav"));const auto encrypted=dir.filePath("encrypted.m4a");
        QProcess encoder;encoder.start(ffmpeg,{"-v","error","-y","-i",source,"-c:a","aac",
            "-encryption_scheme","cenc-aes-ctr","-encryption_key","00000000000000000000000000000000",
            "-encryption_kid","11111111111111111111111111111111",encrypted});
        QVERIFY(encoder.waitForFinished(5000));QCOMPARE(encoder.exitCode(),0);
        SettingsStore settings;settings.setLibraryFolder(dir.filePath("music"));LibraryModel library(nullptr,dir.filePath("library.db"));
        ImportManager importer(&library,&settings);WebLoginManager web(&settings);BrowserAuthManager auth(&settings,&web);AudioTransport transport;
        QFile file(encrypted);QVERIFY(file.open(QIODevice::ReadOnly));transport.payload=file.readAll();
        Track track;track.title="Encrypted fixture";track.source="Яндекс Музыка";track.sourceUrl="https://music.yandex.ru/track/1";track.duration=12000;
        const int id=library.repository().saveTrack(track);QVERIFY(id>0);
        DownloadManager downloads(&settings,&importer,nullptr,&auth,nullptr,&transport);
        downloads.enqueueNative({QVariantMap{{"trackId",id},{"title",track.title},{"url",track.sourceUrl},
            {"durationMs",12000},{"mediaUrl","https://cdn.example/encrypted.m4a"}}},"original");
        QTRY_VERIFY_WITH_TIMEOUT(!downloads.busy(),10000);
        QCOMPARE(downloads.data(downloads.index(0),DownloadManager::StatusRole).toString(),QString("failed"));
        QVERIFY2(downloads.lastError().contains("не декодируется"),qPrintable(downloads.lastError()));
        QVERIFY(library.repository().track(id).filePath.isEmpty());
    }
    void nativeDownloadAttachesAudioAndRejectsFragments() {
        if(QStandardPaths::findExecutable("ffprobe").isEmpty())QSKIP("FFprobe required for audio validation");
        QTemporaryDir dir;SettingsStore settings;settings.setLibraryFolder(dir.filePath("music"));
        LibraryModel library(nullptr,dir.filePath("library.db"));ImportManager importer(&library,&settings);
        WebLoginManager web(&settings);BrowserAuthManager auth(&settings,&web);AudioTransport transport;
        QFile audio(wav(dir.filePath("fixture.wav")));QVERIFY(audio.open(QIODevice::ReadOnly));transport.payload=audio.readAll();
        Track track;track.title="Нативная песня";track.artist="Исполнитель";track.source="VK";track.sourceUrl="https://vk.ru/audio99_1";track.duration=12000;
        const int id=library.repository().saveTrack(track);QVERIFY(id>0);
        const int playlist=library.repository().createPlaylist("Из сервиса");QVERIFY(library.repository().addToPlaylist(playlist,id));
        QVERIFY(library.repository().setRating(id,5));
        QVariantMap item{{"trackId",id},{"title",track.title},{"artist",track.artist},{"url",track.sourceUrl},{"durationMs",12000},{"mediaUrl","https://cdn.example/track.wav"}};
        DownloadManager downloads(&settings,&importer,nullptr,&auth,nullptr,&transport);
        downloads.enqueueNative({item,item},"original");QCOMPARE(downloads.rowCount(),1);
        QTRY_VERIFY_WITH_TIMEOUT(!downloads.busy(),15000);
        QCOMPARE(downloads.data(downloads.index(0),DownloadManager::StatusRole).toString(),QString("succeeded"));
        QCOMPARE(transport.requests,1);QVERIFY(!transport.lastRequest.hasRawHeader("Cookie"));QCOMPARE(transport.lastRequest.rawHeader("Origin"),QByteArray("https://vk.ru"));
        const auto saved=library.repository().track(id);QVERIFY(QFileInfo::exists(saved.filePath));
        QVERIFY(QFileInfo(saved.filePath).fileName().startsWith("Исполнитель — Нативная песня"));
        QCOMPARE(saved.rating,5);QCOMPARE(library.repository().tracks().size(),1);QCOMPARE(library.repository().tracks({},playlist)[0].id,id);
        track.sourceUrl="https://vk.ru/audio99_2";track.title="Только фрагмент";track.duration=45000;
        const int fragmentId=library.repository().saveTrack(track);item["trackId"]=fragmentId;item["url"]=track.sourceUrl;item["title"]=track.title;item["durationMs"]=45000;
        downloads.enqueueNative({item},"original");QTRY_VERIFY_WITH_TIMEOUT(!downloads.busy(),15000);
        QCOMPARE(downloads.data(downloads.index(1),DownloadManager::StatusRole).toString(),QString("failed"));
        QVERIFY(downloads.lastError().contains("фрагмент"));QVERIFY(library.repository().track(fragmentId).filePath.isEmpty());
        track.sourceUrl="https://vk.ru/audio99_3";track.title="Нет доступа";
        const int deniedId=library.repository().saveTrack(track);item["trackId"]=deniedId;item["url"]=track.sourceUrl;transport.status=403;
        downloads.enqueueNative({item},"original");QTRY_VERIFY_WITH_TIMEOUT(!downloads.busy(),15000);
        QCOMPARE(downloads.data(downloads.index(2),DownloadManager::StatusRole).toString(),QString("failed"));
        QVERIFY(downloads.lastError().contains("доступ"));QVERIFY(library.repository().track(deniedId).filePath.isEmpty());
        transport.status=200;transport.delay=500;
        downloads.enqueueNative({item},"original");const int cancelledId=downloads.data(downloads.index(3),DownloadManager::JobIdRole).toInt();
        QTRY_COMPARE(transport.requests,4);
        downloads.cancelJob(cancelledId);QVERIFY(!downloads.busy());
        QCOMPARE(downloads.data(downloads.index(3),DownloadManager::StatusRole).toString(),QString("cancelled"));
        downloads.clearFinished();QCOMPARE(downloads.rowCount(),0);
        track.sourceUrl="https://vk.ru/audio99_4";track.title="Сжатый трек";
        const int compressed=library.repository().saveTrack(track);item["trackId"]=compressed;item["url"]=track.sourceUrl;item["durationMs"]=12000;transport.delay=0;
        downloads.enqueueNative({item},"balanced");QTRY_VERIFY_WITH_TIMEOUT(!downloads.busy(),15000);
        QCOMPARE(downloads.data(downloads.index(0),DownloadManager::StatusRole).toString(),QString("succeeded"));
        QVERIFY(library.repository().track(compressed).filePath.endsWith(".mp3"));
    }

    void ownedBrowserHandlesNavigationScriptsCookiesAndAudio() {
#ifndef Q_OS_WIN
        QTemporaryDir dir;const QString program=dir.filePath("browser");QFile file(program);
        QVERIFY(file.open(QIODevice::WriteOnly));file.write("#!/bin/sh\nexec sleep 30\n");file.close();
        QVERIFY(file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
        DevToolsTransport network;BrowserProtocol protocol;BrowserSession session(nullptr,&network,&protocol,[]{return 9222;});
        QSignalSpy loaded(&session,&BrowserSession::loaded),evaluated(&session,&BrowserSession::evaluated),cookies(&session,&BrowserSession::cookiesReceived),audio(&session,&BrowserSession::audioRequested),failed(&session,&BrowserSession::failed);
        session.open(program,"vk",QUrl("https://vk.ru/audios99"),QJsonArray{QJsonObject{{"name","remixsid6"},{"value","fixture"},{"domain",".vk.ru"},{"expires","1999999999"}}});
        QTRY_VERIFY_WITH_TIMEOUT(loaded.count()>0 && cookies.count()>0,4000);QVERIFY(failed.isEmpty());
        QCOMPARE(session.userAgent(),QString("Test Chromium"));
        bool setCookies=false;
        for(const auto &call:protocol.commands)if(call.first=="Network.setCookie") {
            setCookies=true;const auto cookie=call.second;
            QVERIFY(cookie.value("expires").isDouble());QCOMPARE(cookie.value("name").toString(),QString("remixsid6"));
        }
        QVERIFY(setCookies);
        session.evaluate(42,"document.title");QTRY_COMPARE(evaluated.count(),1);QCOMPARE(evaluated[0][0].toInt(),42);
        const auto tracks=WebLoginManager::parseHarvest(evaluated[0][1].toString().toUtf8(),"vk");QCOMPARE(tracks.size(),1);QCOMPARE(tracks[0].sourceUrl,QString("https://vk.ru/audio99_10"));
        session.evaluate(43,"navigation-fixture");QTRY_COMPARE(evaluated.count(),2);QCOMPARE(evaluated[1][0].toInt(),43);QVERIFY(!evaluated[1][1].isValid());QVERIFY(failed.isEmpty());
        emit protocol.eventReceived("Network.requestWillBeSent",{{"type","Media"},{"request",QJsonObject{{"url","https://cdn.example/song.mp3"}}}});QCOMPARE(audio.count(),0);
        session.evaluate(44,"const requested = {}",true);QTRY_COMPARE(audio.count(),1);
        emit protocol.eventReceived("Network.responseReceived",{{"response",QJsonObject{{"url","https://cdn.example/stream?id=42"},{"mimeType","audio/mp4"}}}});QCOMPARE(audio.count(),2);
        emit protocol.eventReceived("Network.responseReceived",{{"response",QJsonObject{{"url","https://cdn.example/login"},{"mimeType","text/html"}}}});QCOMPARE(audio.count(),2);
        const auto before=loaded.count();
        emit protocol.eventReceived("Network.loadingFailed",{{"type","Document"},{"requestId","unrelated-iframe"},{"errorText","net::ERR_NAME_NOT_RESOLVED"}});QCOMPARE(loaded.count(),before);
        session.hide();QTest::qWait(20);session.open(program,"vk",QUrl("https://vk.ru/audio99_10"),{});
        QTRY_VERIFY(loaded.count()>before);QVERIFY(failed.isEmpty());
        const QString alternate=dir.filePath("other-browser");QVERIFY(QFile::copy(program,alternate));
        const auto countInitializations=[&]{int count=0;for(const auto &call:protocol.commands)if(call.first=="Network.enable")++count;return count;};
        const int previous=countInitializations();session.open(alternate,"vk",QUrl("https://vk.ru/audios99"),{});QTRY_VERIFY(countInitializations()>previous);QVERIFY(failed.isEmpty());session.stop();
        BrowserSession missing(nullptr,&network,&protocol,[]{return 9222;});QSignalSpy failure(&missing,&BrowserSession::failed);
        missing.open(dir.filePath("missing-browser"),"vk",QUrl("https://vk.ru/"),{});QTRY_COMPARE_WITH_TIMEOUT(failure.count(),1,3000);
#endif
    }

    void webImportRunsFromLoginThroughDatabaseAndDownloadedFile() {
#ifndef Q_OS_WIN
        QTemporaryDir dir;const QString program=dir.filePath("browser");QFile executable(program);QVERIFY(executable.open(QIODevice::WriteOnly));executable.write("#!/bin/sh\nexec sleep 30\n");executable.close();QVERIFY(executable.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
        SettingsStore settings;settings.setLibraryFolder(dir.filePath("music"));DevToolsTransport network;BrowserProtocol protocol;
        WebLoginManager web(&settings,nullptr,[&](QObject *parent){return new BrowserSession(parent,&network,&protocol,[]{return 9222;});});
        // Select the app-owned executable through the normal public browser-selection path.
        const auto oldPath=qgetenv("PATH");const auto restore=qScopeGuard([&]{qputenv("PATH",oldPath);});
        const auto alias=dir.filePath("brave");QVERIFY(QFile::copy(program,alias));QVERIFY(QFile::setPermissions(alias,QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));qputenv("PATH",dir.path().toUtf8()+":"+oldPath);
        QSignalSpy loggedIn(&web,&WebLoginManager::loginDone);web.openNative("VK");QTRY_VERIFY_WITH_TIMEOUT(!loggedIn.isEmpty(),4000);QVERIFY(loggedIn[0][1].toBool());QVERIFY(web.vkLoggedIn());QVERIFY(web.nativeProfile()==nullptr);
        LibraryModel library(nullptr,dir.filePath("library.db"));LibraryExchange exchange(&library,&settings,nullptr,nullptr,&web);QSignalSpy completed(&exchange,&LibraryExchange::completed);
        exchange.importUrl("https://vk.ru/audios99");QTRY_VERIFY_WITH_TIMEOUT(!completed.isEmpty(),12000);QVERIFY(!exchange.busy());QCOMPARE(library.repository().count(),1);QCOMPARE(exchange.importedTracks().size(),1);
        const int id=exchange.importedTracks()[0].toMap().value("trackId").toInt();QVERIFY(id>0);QVERIFY(library.repository().setRating(id,5));
        ImportManager importer(&library,&settings);BrowserAuthManager auth(&settings,&web);AudioTransport audio;
        QFile source(wav(dir.filePath("audio.wav")));QVERIFY(source.open(QIODevice::ReadOnly));audio.payload=source.readAll();
        audio.responseStatuses={403,200};
        DownloadManager downloads(&settings,&importer,nullptr,&auth,nullptr,&audio);downloads.enqueueNative(exchange.importedTracks(),"original");QTRY_VERIFY_WITH_TIMEOUT(!downloads.busy(),6000);
        QCOMPARE(downloads.data(downloads.index(0),DownloadManager::StatusRole).toString(),QString("succeeded"));QCOMPARE(audio.requests,2);QVERIFY(QFileInfo::exists(library.repository().track(id).filePath));QCOMPARE(library.repository().track(id).rating,5);QCOMPARE(library.repository().tracks({},completed[0][0].toInt())[0].id,id);QVERIFY(web.nativeProfile()==nullptr);
        bool gesture=false;for(const auto &call:protocol.commands)if(call.first=="Runtime.evaluate" && call.second.value("expression").toString().contains("const requested"))gesture=call.second.value("userGesture").toBool();QVERIFY(gesture);web.logout("vk");
#endif
    }
    void browserSelectionAndVkSessionMarkers() {
        const QJsonArray pages{QJsonObject{{"type","page"},{"url","https://vk.ru/audios99"},{"webSocketDebuggerUrl","ws://attacker.example:9222/"}},QJsonObject{{"type","page"},{"url","https://vk.ru/audios99"},{"webSocketDebuggerUrl","ws://127.0.0.1:9222/devtools/page/1"}}};
        QCOMPARE(BrowserSession::choosePage(pages,"vk",9222).path(),QString("/devtools/page/1"));
        QVERIFY(BrowserSession::choosePage(pages,"vk",9223).isEmpty());
        QVERIFY(BrowserSession::failureText("net::ERR_NAME_NOT_RESOLVED").contains("DNS"));
        QVERIFY(WebLoginManager::hasSessionCookie({{"remixsid6","fixture",".vk.ru","/",0,true,true}},"vk"));
        QVERIFY(!WebLoginManager::hasSessionCookie({{"remixsid6","fixture",".vk.ru.evil.test","/",0,true,true}},"vk"));
    }
    void toolsAreCheckedByExecutionAndFfmpegUsesSingleDash() {
#ifndef Q_OS_WIN
        QTemporaryDir dir;const auto previous=qgetenv("PATH");const auto restore=qScopeGuard([&]{qputenv("PATH",previous);});
        auto write=[&](const QString &name,const QByteArray &body){QFile file(dir.filePath(name));if(!file.open(QIODevice::WriteOnly))return false;file.write(body);file.close();return file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner);};
        QVERIFY(write("ffmpeg","#!/bin/sh\n[ \"$1\" = -version ] || exit 1\necho 'ffmpeg version fixture'\n"));
        QVERIFY(write("ffprobe","#!/bin/sh\n[ \"$1\" = -version ] || exit 1\necho 'ffprobe version fixture'\n"));
        QVERIFY(write("yt-dlp","#!/bin/sh\necho 2026.11.05\n"));qputenv("PATH",dir.path().toUtf8());
        QVERIFY(write("node","#!/bin/sh\necho v22.12.0\n"));
        SettingsStore settings;settings.setYtDlpPath(dir.filePath("yt-dlp"));DependencyManager tools(&settings);
        QVERIFY(tools.ffmpegAvailable());QVERIFY(tools.ytDlpAvailable());QVERIFY(tools.runtimeReady());QVERIFY(tools.ffmpegVersion().contains("ffmpeg version"));
        QVERIFY(write("ffmpeg","#!/bin/sh\nexit 1\n"));tools.refresh();QVERIFY(!tools.ffmpegAvailable());QVERIFY(!tools.runtimeReady());
#endif
    }
    void previewCancellationIsNotReportedAsCrash() {
#ifndef Q_OS_WIN
        QTemporaryDir dir;const QString executable=dir.filePath("yt-dlp");QFile file(executable);QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("#!/bin/sh\nif [ \"$1\" = --version ]; then echo 2026.11.05; exit 0; fi\nexec sleep 30\n");file.close();QVERIFY(file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner));
        SettingsStore settings;settings.setYtDlpPath(executable);DependencyManager tools(&settings);DownloadManager downloads(nullptr,nullptr,nullptr,nullptr);ImportPreviewer preview(&settings,&tools,nullptr,&downloads);
        preview.searchYoutube("fixture","original");QVERIFY(preview.busy());preview.cancel();QTRY_VERIFY_WITH_TIMEOUT(!preview.busy(),3000);QVERIFY(preview.lastError().contains("отменено"));QVERIFY(!preview.lastError().contains("аварийно"));
#endif
    }
    void appearancePersistsAndBoundsValues() {
        SettingsStore settings;settings.setColorScheme("lagoon");settings.setCustomAccent("#ee99aa");settings.setSphereStyle("liquid");settings.setMotionEnabled(false);settings.setAtmosphereIntensity(100);
        SettingsStore restored;QCOMPARE(restored.colorScheme(),QString("lagoon"));QCOMPARE(restored.customAccent(),QString("#ee99aa"));QCOMPARE(restored.sphereStyle(),QString("liquid"));QVERIFY(!restored.motionEnabled());QCOMPARE(restored.atmosphereIntensity(),1.5);
        settings.setColorScheme("invalid");QCOMPARE(settings.colorScheme(),QString("lagoon"));
        settings.setCustomAccent("invalid");QCOMPARE(settings.customAccent(),QString("#ee99aa"));
        settings.setAtmosphereIntensity(-1);QCOMPARE(settings.atmosphereIntensity(),0.0);
        settings.resetPreferences();QCOMPARE(settings.colorScheme(),QString("aurora"));QVERIFY(settings.motionEnabled());
    }
    void downloadServiceRecognizesYtSearch() {
        DownloadManager manager(nullptr,nullptr,nullptr,nullptr);
        QCOMPARE(manager.serviceForUrl("ytsearch1:Queen"),QString("YouTube"));
        QCOMPARE(manager.serviceForUrl("ytsearch:some query"),QString("YouTube"));
        QCOMPARE(manager.serviceForUrl("https://vk.com/audio99_10"),QString("VK"));
        QCOMPARE(manager.serviceForUrl("https://music.yandex.ru/users/1/playlists/2"),QString("Яндекс Музыка"));
    }
};
int main(int argc,char **argv) {
    QTemporaryDir env; qputenv("XDG_DATA_HOME",env.filePath("data").toUtf8());qputenv("XDG_CONFIG_HOME",env.filePath("config").toUtf8());qputenv("XDG_STATE_HOME",env.filePath("state").toUtf8());qputenv("AURORA_HEADLESS","1");
    QApplication app(argc,argv);QCoreApplication::setOrganizationName("AuroraTests");QCoreApplication::setApplicationName("Library");
    QSettings().setValue("libraryFolder",env.filePath("music"));LibraryTests test;return QTest::qExec(&test,argc,argv);
}
#include "tst_library.moc"
