#include "auth/browserauthmanager.h"
#include "downloadmanager.h"
#include "downloads/dependencymanager.h"
#include "downloads/importpreviewer.h"
#include "importmanager.h"
#include "librarymodel.h"
#include "playbackcontroller.h"
#include "settingsstore.h"
#include "storage/storagemanager.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTimer>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Aurora"));
    QCoreApplication::setApplicationName(QStringLiteral("AuroraPlayer"));
    QCoreApplication::setApplicationVersion(QStringLiteral("3.0"));

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption smokeOption(QStringLiteral("smoke-test"), QStringLiteral("Exit after validating application startup"));
    QCommandLineOption importOption(QStringLiteral("import"), QStringLiteral("Import an audio file at startup"), QStringLiteral("path"));
    QCommandLineOption screenshotOption(QStringLiteral("screenshot"), QStringLiteral("Save a screenshot during smoke testing"), QStringLiteral("path"));
    QCommandLineOption setupToolsOption(QStringLiteral("setup-tools"), QStringLiteral("Install managed downloader dependencies and exit"));
    parser.addOption(smokeOption);
    parser.addOption(importOption);
    parser.addOption(screenshotOption);
    parser.addOption(setupToolsOption);
    parser.process(app);

    SettingsStore settings;
    LibraryModel library;
    ImportManager importer(&library, &settings);
    PlaybackController playback(&library, &settings);
    DependencyManager dependencies(&settings);
    BrowserAuthManager browserAuth(&settings);
    DownloadManager downloads(&settings, &importer, &dependencies, &browserAuth);
    ImportPreviewer previewer(&settings, &dependencies, &browserAuth, &downloads);
    StorageManager storage(&settings);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("libraryModel"), &library);
    engine.rootContext()->setContextProperty(QStringLiteral("importManager"), &importer);
    engine.rootContext()->setContextProperty(QStringLiteral("player"), &playback);
    engine.rootContext()->setContextProperty(QStringLiteral("settingsStore"), &settings);
    engine.rootContext()->setContextProperty(QStringLiteral("downloadDependencies"), &dependencies);
    engine.rootContext()->setContextProperty(QStringLiteral("browserAuth"), &browserAuth);
    engine.rootContext()->setContextProperty(QStringLiteral("downloadManager"), &downloads);
    engine.rootContext()->setContextProperty(QStringLiteral("importPreviewer"), &previewer);
    engine.rootContext()->setContextProperty(QStringLiteral("storageManager"), &storage);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("AuroraPlayer"), QStringLiteral("Main"));
    if (parser.isSet(setupToolsOption)) {
        QObject::connect(&dependencies, &DependencyManager::busyChanged, &app, [&app, &dependencies] {
            if (!dependencies.busy()) {
                QTimer::singleShot(0, &app, [&app, &dependencies] {
                    qInfo().noquote() << dependencies.statusText()
                                      << (dependencies.lastError().isEmpty() ? QString() : dependencies.lastError());
                    app.exit(dependencies.runtimeReady() ? 0 : 1);
                });
            }
        });
        QTimer::singleShot(0, &dependencies, &DependencyManager::installOrUpdate);
    }
    if (parser.isSet(screenshotOption)) {
        auto *window = engine.rootObjects().isEmpty() ? nullptr : qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        if (window && qEnvironmentVariableIsSet("AURORA_SMOKE_COMPACT"))
            window->resize(390, 760);
        if (window && qEnvironmentVariableIsSet("AURORA_SMOKE_PAGE"))
            window->setProperty("currentPage", qEnvironmentVariableIntValue("AURORA_SMOKE_PAGE"));
    }

    if (parser.isSet(importOption))
        importer.importPaths({QFileInfo(parser.value(importOption)).absoluteFilePath()});
    if (parser.isSet(smokeOption)) {
        const bool expectPlayback = library.count() > 0;
        if (expectPlayback) {
            settings.setVolume(0.0);
            playback.playIndex(0);
        }
        QTimer::singleShot(1500, &app, [&app, &library, &playback, &engine, &parser, &screenshotOption, expectPlayback] {
            const QString error = !library.lastError().isEmpty() ? library.lastError() : playback.lastError();
            const bool playbackOk = !expectPlayback || playback.position() > 0;
            bool screenshotOk = true;
            if (parser.isSet(screenshotOption)) {
                auto *window = engine.rootObjects().isEmpty() ? nullptr : qobject_cast<QQuickWindow *>(engine.rootObjects().first());
                screenshotOk = window && window->grabWindow().save(parser.value(screenshotOption));
            }
            qInfo().noquote() << QStringLiteral("Aurora Player smoke test: %1 tracks; position: %2 ms; recommendations: %3; error: %4")
                                     .arg(library.count())
                                     .arg(playback.position())
                                     .arg(library.recommendations().size())
                                     .arg(error.isEmpty() ? QStringLiteral("none") : error);
            app.exit(error.isEmpty() && playbackOk && screenshotOk ? 0 : 1);
        });
    }
    return app.exec();
}
