#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QStandardPaths>
#include <QDir>
#include "database_manager.h"

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("DesktopGeekDashboard");
    app.setApplicationName("DesktopGeekDashboard");

    // Initialize database
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    QString dbPath = dataPath + "/config.db";
    DatabaseManager::instance().openDatabase(dbPath);

    QQmlApplicationEngine engine;

    // QML_ELEMENT / QML_SINGLETON will automatically register WindowManager
    // if the module is correctly imported.

    const QUrl url(u"qrc:/GeekDashboard/qml/Main.qml"_s);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    int ret = app.exec();

    DatabaseManager::instance().closeDatabase();
    return ret;
}