#include "MarmosetItem.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    QQuickWindow::setSceneGraphBackend(QStringLiteral("opengl"));
    QGuiApplication app(argc, argv);
    qmlRegisterType<MarmosetItem>("MiniMarmoset", 1, 0, "MarmosetView");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *object, const QUrl &objectUrl) {
                         if (!object && objectUrl == url)
                             QCoreApplication::exit(-1);
                     }, Qt::QueuedConnection);
    engine.load(url);
    return app.exec();
}
