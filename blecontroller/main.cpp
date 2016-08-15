#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "singlescanner.h"
#include "spidercenter.h"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    MyQmlController spider;

    QQmlApplicationEngine engine;
    QQmlContext * ctx = engine.rootContext();
    ctx->setContextProperty("cstController",&spider);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    return app.exec();
}
