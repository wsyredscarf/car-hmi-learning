#include <QGuiApplication>
#include <QQmlApplicationEngine>
// 引入我们自己写的车辆业务类
#include "cardata.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 实例化C++车辆数据对象，全局唯一，所有QML页面共用
    CarData carBiz;

    QQmlApplicationEngine engine;
    // 核心步骤：把C++对象注册到QML全局，QML中通过标识符g_car直接访问
    engine.rootContext()->setContextProperty("g_car", &carBiz);

    // 加载界面qml文件
    const QUrl url(u"qrc:/qt_car_demo/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}