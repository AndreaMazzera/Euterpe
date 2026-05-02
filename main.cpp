#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCoreApplication>
#include <QDebug>

#include "song_model.h"
#include "audio_player.h"
#include "playback_controller.h"
#include "playback_filter_model.h"
#include "metadata_service.h"

int main(int argc, char *argv[])
{
    // Configures how the application handles non-integer scale factors for High DPI displays.
    // By using PassThrough, i prevent UI blurring or rounding artifacts on devices with fractional scaling (e.g., 1.5x).
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QGuiApplication app(argc, argv);
    app.setOrganizationName("AndreaMazzera");
    app.setApplicationName("Euterpe");

    QQmlApplicationEngine engine;

    SongModel songModel;
    AudioPlayer audioPlayer;
    PlaybackController controller;
    PlaybackFilterModel playbackFilterModel;
    MetadataService *metadataService = new MetadataService(&app);

    controller.setModel(&songModel);
    controller.setPlayer(&audioPlayer);
    playbackFilterModel.setSourceModel(&songModel);

    QQmlContext *rootContext = engine.rootContext();
    rootContext->setContextProperty("songModel", &songModel);
    rootContext->setContextProperty("playbackController", &controller);
    rootContext->setContextProperty("playbackFilterModel", &playbackFilterModel);
    rootContext->setContextProperty("audioPlayer", &audioPlayer);

    qmlRegisterSingletonInstance("Euterpe", 1, 0, "MetadataService", metadataService);

#ifdef Q_OS_ANDROID
    // I use QueuedConnection to allow the UI to appear before blocking the thread with the scan
    QObject::connect(&songModel, &SongModel::permissionsGranted, &songModel, &SongModel::scanMusicFolder, Qt::QueuedConnection);

    // Asynchronous startup: first I load the QML engine, then I ask for permissions
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &songModel, [&songModel](QObject *obj, const QUrl &) {
        if (obj) songModel.requestAndroidPermissions();
    });
#else
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &songModel, [&songModel](QObject *obj, const QUrl &) {
        if (obj) songModel.scanMusicFolder();
    });
#endif

    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, [](){ QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("Euterpe", "Main");

    return app.exec();
}
