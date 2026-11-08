#include "integration/namelessbridge.h"
#ifdef AURORA_MPRIS
#include "integration/mpris.h"
#endif
#include "auth/browserauthmanager.h"
#include "auth/oauthmanager.h"
#include "auth/webloginmanager.h"
#include "downloadmanager.h"
#include "downloads/dependencymanager.h"
#include "downloads/importpreviewer.h"
#include "downloads/audiosources.h"
#include "downloads/musicsearch.h"
#include "importmanager.h"
#include "librarymodel.h"
#include "homefeed.h"
#include "libraryexchange.h"
#include "playbackcontroller.h"
#include "settingsstore.h"
#include "storage/storagemanager.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QDir>
#include <QSaveFile>
#include <QStandardPaths>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTimer>
#include <QTextStream>
#include <QQuickStyle>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>

// Точка входа приложения: инициализация Qt/WebEngine, разбор аргументов командной строки,
// создание всех сервисов и моделей, загрузка QML-интерфейса и поддержка smoke-тестов.

int main(int argc, char* argv[])
{
    QElapsedTimer startup;
    startup.start();
    // Собираем замеры этапов запуска для диагностического файла startup.json.
    QJsonObject startupTimes;
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QtWebEngineQuick::initialize();
    QQuickStyle::setStyle("Basic");
    // Режим без реального рендера (для headless-тестов).
    if (qEnvironmentVariableIsSet("AURORA_TEST_NULL_RHI"))
        QQuickWindow::setGraphicsApi(QSGRendererInterface::Null);
    QApplication app(argc, argv);
    QGuiApplication::setDesktopFileName("org.nameless.Aurora");
    QCoreApplication::setOrganizationName(QStringLiteral("Aurora"));
    QCoreApplication::setApplicationName(QStringLiteral("AuroraPlayer"));
    QCoreApplication::setApplicationVersion(QStringLiteral("4.0"));
    startupTimes["qtReadyMs"] = startup.elapsed();

    QCommandLineParser parser;
    parser.addHelpOption();
    // Режимы командной строки: smoke-тест, импорт файла, скриншот, установка/проверка загрузочных инструментов.
    QCommandLineOption smokeOption(QStringLiteral("smoke-test"),
                                   QStringLiteral("Exit after validating application startup"));
    QCommandLineOption importOption(QStringLiteral("import"), QStringLiteral("Import an audio file at startup"),
                                    QStringLiteral("path"));
    QCommandLineOption screenshotOption(
        QStringLiteral("screenshot"), QStringLiteral("Save a screenshot during smoke testing"), QStringLiteral("path"));
    QCommandLineOption setupToolsOption(QStringLiteral("setup-tools"),
                                        QStringLiteral("Install managed downloader dependencies and exit"));
    QCommandLineOption checkToolsOption(QStringLiteral("check-tools"),
                                        QStringLiteral("Check installed tools without opening the player"));
    parser.addOption(checkToolsOption);
    parser.addOption(smokeOption);
    parser.addOption(importOption);
    parser.addOption(screenshotOption);
    parser.addOption(setupToolsOption);
    parser.process(app);

    if (parser.isSet(checkToolsOption)) {
        // Печатаем сведения об окружении (версии инструментов, браузеры) в JSON и выходим.
        SettingsStore settings;
        DependencyManager tools(&settings);
        QStringList browsers;
        for (const auto& path : WebLoginManager::detectBrowserExecutables())
            browsers.append(QFileInfo(path).completeBaseName());
        QJsonObject result{{"qt", qVersion()},
                           {"ytDlp", tools.ytDlpVersion()},
                           {"spotdl", tools.spotdlVersion()},
                           {"ffmpeg", tools.ffmpegVersion()},
                           {"audioToolsReady", tools.ffmpegAvailable()},
                           {"youtubeReady", tools.runtimeReady()},
                           {"browsers", QJsonArray::fromStringList(browsers)}};
        QTextStream(stdout) << QJsonDocument(result).toJson(QJsonDocument::Indented);
        return tools.runtimeReady() ? 0 : 1;
    }
    if (parser.isSet(setupToolsOption)) {
        // Устанавливаем/обновляем загрузочные инструменты и завершаемся по готовности.
        SettingsStore settings;
        DependencyManager dependencies(&settings);
        QObject::connect(&dependencies, &DependencyManager::busyChanged, &app, [&] {
            if (!dependencies.busy())
                QTimer::singleShot(0, &app, [&] {
                    QTextStream(stderr) << dependencies.statusText() << " " << dependencies.lastError() << Qt::endl;
                    app.exit(dependencies.runtimeReady() && dependencies.lastError().isEmpty() ? 0 : 1);
                });
        });
        QTimer::singleShot(0, &dependencies, &DependencyManager::installOrUpdate);
        return app.exec();
    }
    if (parser.isSet(smokeOption))
        qputenv("AURORA_HEADLESS", "1");
    // Создаём все сервисы в порядке их зависимостей.
    NamelessBridge nameless;
    SettingsStore settings;
    startupTimes["settingsReadyMs"] = startup.elapsed();
    LibraryModel library;
    startupTimes["libraryReadyMs"] = startup.elapsed();
    startupTimes["libraryTracks"] = library.count();
    ImportManager importer(&library, &settings);
    PlaybackController playback(&library, &settings);
    startupTimes["playbackReadyMs"] = startup.elapsed();
    DependencyManager dependencies(&settings, nullptr, true);
    startupTimes["toolsScheduledMs"] = startup.elapsed();
    WebLoginManager webLogin(&settings);
    BrowserAuthManager browserAuth(&settings, &webLogin);
    DownloadManager downloads(&settings, &importer, &dependencies, &browserAuth);
    AudioSources audioSources(&library, &downloads, &dependencies);
    MusicSearch musicSearch(&library, &downloads, &dependencies, &webLogin);
    QObject::connect(&musicSearch, &MusicSearch::playRequested, &playback, &PlaybackController::playStream);
    QObject::connect(&musicSearch, &MusicSearch::playLibraryRequested, &playback, &PlaybackController::playTrackId);
    ImportPreviewer previewer(&settings, &dependencies, &browserAuth, &downloads);
    startupTimes["browserReadyMs"] = startup.elapsed();
    StorageManager storage(&settings);
    startupTimes["storageScheduledMs"] = startup.elapsed();
    OAuthManager accounts(&settings);
    LibraryExchange exchange(&library, &settings, &dependencies, &accounts, &webLogin);
    AudioSources homePreview(&library, &downloads, &dependencies, nullptr, {}, true);
    HomeFeed homeFeed(&library, &accounts, &webLogin, &playback, nullptr, nullptr, &homePreview);
    QObject::connect(&homeFeed, &HomeFeed::sourceRequested, &audioSources, &AudioSources::openTrack);
    startupTimes["servicesReadyMs"] = startup.elapsed();
    QObject::connect(&exchange, &LibraryExchange::audioImportRequested, &downloads, [&](const QVariantList& tracks) {
        downloads.enqueueNative(tracks, settings.quality());
    });

    int qmlWarnings = 0;
    QQmlApplicationEngine engine;
    // Сохраняем диагностику запуска в startup.json (и в stderr при включённом профилировании).
    const auto saveStartup = [&] {
        const auto directory =
            QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("diagnostics");
        if (!QDir().mkpath(directory))
            return;
        QSaveFile file(QDir(directory).filePath("startup.json"));
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(startupTimes).toJson());
            file.commit();
        }
        if (qEnvironmentVariableIsSet("AURORA_PROFILE_STARTUP"))
            QTextStream(stderr) << QJsonDocument(startupTimes).toJson(QJsonDocument::Compact) << Qt::endl;
    };
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app, [&](QObject* object, const QUrl&) {
        if (auto* window = qobject_cast<QQuickWindow*>(object))
            QObject::connect(
                window, &QQuickWindow::frameSwapped, &app,
                [&] {
                    // Фиксируем момент отрисовки первого кадра.
                    if (startupTimes.contains("firstFrameMs"))
                        return;
                    startupTimes["firstFrameMs"] = startup.elapsed();
                    saveStartup();
                },
                Qt::QueuedConnection);
    });
    if (parser.isSet(smokeOption))
        QObject::connect(&engine, &QQmlEngine::warnings, &app, [&](const QList<QQmlError>& errors) {
            for (const auto& error : errors) {
                ++qmlWarnings;
                QTextStream(stderr) << error.toString() << Qt::endl;
            }
        });
    // Регистрируем сервисы в контексте QML для доступа из интерфейса.
    engine.rootContext()->setContextProperty(QStringLiteral("libraryExchange"), &exchange);
    engine.rootContext()->setContextProperty(QStringLiteral("namelessBridge"), &nameless);
    engine.rootContext()->setContextProperty(QStringLiteral("libraryModel"), &library);
    engine.rootContext()->setContextProperty(QStringLiteral("homeFeed"), &homeFeed);
    engine.rootContext()->setContextProperty(QStringLiteral("importManager"), &importer);
    engine.rootContext()->setContextProperty(QStringLiteral("player"), &playback);
    engine.rootContext()->setContextProperty(QStringLiteral("settingsStore"), &settings);
    engine.rootContext()->setContextProperty(QStringLiteral("downloadDependencies"), &dependencies);
    engine.rootContext()->setContextProperty(QStringLiteral("browserAuth"), &browserAuth);
    engine.rootContext()->setContextProperty(QStringLiteral("downloadManager"), &downloads);
    engine.rootContext()->setContextProperty(QStringLiteral("audioSources"), &audioSources);
    engine.rootContext()->setContextProperty(QStringLiteral("musicSearch"), &musicSearch);
    engine.rootContext()->setContextProperty(QStringLiteral("importPreviewer"), &previewer);
    engine.rootContext()->setContextProperty(QStringLiteral("storageManager"), &storage);
    engine.rootContext()->setContextProperty(QStringLiteral("accounts"), &accounts);
    engine.rootContext()->setContextProperty(QStringLiteral("webLogin"), &webLogin);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] {
            QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("AuroraPlayer"), QStringLiteral("Main"));
    startupTimes["qmlReadyMs"] = startup.elapsed();
    QTimer::singleShot(0, &app, [&] {
        startupTimes["eventLoopMs"] = startup.elapsed();
        saveStartup();
    });
#ifdef AURORA_MPRIS
    // Подключаемся к D-Bus после загрузки интерфейса, когда окно уже известно.
    registerMpris(&app, &playback,
                  engine.rootObjects().isEmpty() ? nullptr : qobject_cast<QWindow*>(engine.rootObjects().first()));
#endif
    if (parser.isSet(screenshotOption)) {
        // Настраиваем окно для скриншот-теста (состояние прослушивания, размер, страница).
        auto* window =
            engine.rootObjects().isEmpty() ? nullptr : qobject_cast<QQuickWindow*>(engine.rootObjects().first());
        if (window && qEnvironmentVariableIsSet("AURORA_SMOKE_LISTENING"))
            window->setProperty("listening", true);
        if (window && qEnvironmentVariableIsSet("AURORA_SMOKE_COMPACT"))
            window->resize(390, 760);
        if (window && qEnvironmentVariableIsSet("AURORA_SMOKE_PAGE"))
            window->setProperty("currentPage", qEnvironmentVariableIntValue("AURORA_SMOKE_PAGE"));
    }

    if (parser.isSet(importOption)) {
        QStringList paths;
        for (const auto& path : parser.values(importOption))
            paths.append(QFileInfo(path).absoluteFilePath());
        importer.importPaths(paths);
    }
    // Smoke-тест: запускает воспроизведение, выполняет заданные действия UI и проверяет результат.
    auto runSmoke = [&] {
        const auto playable = library.playableIds();
        const bool expectPlayback = !playable.isEmpty();
        if (expectPlayback) {
            settings.setVolume(0.0);
            playback.playTrackId(playable.first());
        }
        if (qEnvironmentVariableIsSet("AURORA_SMOKE_ACTION"))
            QTimer::singleShot(250, &app, [&] {
                QObject* window = engine.rootObjects().isEmpty() ? nullptr : engine.rootObjects().first();
                if (!window)
                    return;
                const QString action = qEnvironmentVariable("AURORA_SMOKE_ACTION");
                QVariant result;
                // Вызываем QML-метод exerciseAction для проверки интерактивных сценариев.
                const bool invoked = QMetaObject::invokeMethod(window, "exerciseAction", Q_RETURN_ARG(QVariant, result),
                                                               Q_ARG(QVariant, QVariant(action)));
                if (!invoked || !result.toBool()) {
                    ++qmlWarnings;
                    QTextStream(stderr) << "UI action failed: " << action << Qt::endl;
                }
            });
        QTimer::singleShot(
            qBound(1500, qEnvironmentVariableIntValue("AURORA_SMOKE_MS"), 60000), &app,
            [&app, &library, &playback, &engine, &parser, &screenshotOption, &qmlWarnings, expectPlayback] {
                const QString error = !library.lastError().isEmpty() ? library.lastError() : playback.lastError();
                const bool playbackOk = !expectPlayback || playback.position() > 0;
                bool screenshotOk = true;
                if (parser.isSet(screenshotOption)) {
                    auto* window = engine.rootObjects().isEmpty()
                                       ? nullptr
                                       : qobject_cast<QQuickWindow*>(engine.rootObjects().first());
                    screenshotOk = window && window->grabWindow().save(parser.value(screenshotOption));
                }
                if (qEnvironmentVariableIsSet("AURORA_TEST_NULL_RHI")) {
                    // Проверяем, что шейдер скомпилировался даже при нулевом RHI-рендере.
                    auto* window = engine.rootObjects().isEmpty() ? nullptr : engine.rootObjects().first();
                    const auto* shader = window ? window->findChild<QObject*>("auroraOrbShader") : nullptr;
                    const bool valid = shader && shader->property("validShader").toBool();
                    QTextStream(stderr) << "Shader program valid: " << valid << Qt::endl;
                    if (!valid)
                        ++qmlWarnings;
                }
                QTextStream(stderr)
                    << QStringLiteral(
                           "Aurora Player smoke test: %1 tracks; position: %2 ms; recommendations: %3; error: %4")
                           .arg(library.count())
                           .arg(playback.position())
                           .arg(library.recommendations().size())
                           .arg(error.isEmpty() ? QStringLiteral("none") : error)
                    << "; QML warnings: " << qmlWarnings << Qt::endl;
                app.exit(error.isEmpty() && playbackOk && screenshotOk && qmlWarnings == 0 ? 0 : 1);
            });
    };
    if (parser.isSet(smokeOption)) {
        if (importer.busy())
            QObject::connect(&importer, &ImportManager::importCompleted, &app, [&](int, int) {
                runSmoke();
            });
        else
            runSmoke();
    }
    return app.exec();
}
