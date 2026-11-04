#include "Logger.h"

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QTimer>
#include <QImage>
#include <QDebug>

int main(int argc, char *argv[])
{
    // 在创建任何 Qt Quick 窗口前，指定场景图使用 OpenGL 后端。
    QQuickWindow::setSceneGraphBackend(QStringLiteral("opengl"));

    // 创建 GUI 应用，管理窗口系统和事件循环；应用名称也用于标识日志。
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Mini-Marmoset"));
    app.setOrganizationName(QStringLiteral("OpenGL Lab"));

    // 初始化可执行文件旁的 logs/，并接管 Qt/QML 日志。
    // 初始化失败时继续启动，警告仍交给 Qt 原有的日志处理器输出。
    if (!Logger::initialize())
        qWarning() << "Unable to initialize file logging";
    Logger::info(QStringLiteral("Starting Mini-Marmoset"));

    // 在加载 QML 控件之前统一设置 Fusion 样式。
    QQuickStyle::setStyle(QStringLiteral("Fusion"));

    // 声明并解析命令行选项：初始模块、截图路径、窄窗口模式。
    // addHelpOption() 提供 --help；process() 处理帮助和无效选项。
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("OpenGL learning workbench"));
    parser.addHelpOption();
    parser.addOption({QStringLiteral("module"), QStringLiteral("Initial module index (0-3)."), QStringLiteral("index")});
    parser.addOption({QStringLiteral("capture"), QStringLiteral("Save a UI screenshot and exit."), QStringLiteral("file")});
    parser.addOption({QStringLiteral("compact"), QStringLiteral("Open at 1080 x 720." )});
    parser.process(app);

    // 默认进入 PBR；索引 0～3 对应 PBR、Deferred、SSAO、GPU Simulation。
    // 同时检查转换是否成功和索引范围，非法输入返回退出码 2。
    int initialModule = 0;
    if (parser.isSet(QStringLiteral("module"))) {
        bool valid = false;
        initialModule = parser.value(QStringLiteral("module")).toInt(&valid);
        if (!valid || initialModule < 0 || initialModule > 3) {
            qCritical() << "--module must be an integer from 0 to 3";
            return 2;
        }
    }

    // 在加载界面前向 QML 根上下文传入启动参数，使界面初始化即可读取。
    // 后续需要注册自定义 QML 类型时，也应在 engine.load() 之前完成。
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("initialModuleIndex"), initialModule);
    engine.rootContext()->setContextProperty(QStringLiteral("compactWindow"), parser.isSet(QStringLiteral("compact")));
    // 从编译进程序的 Qt 资源加载主界面；未创建根对象表示加载失败，返回 1。
    engine.load(QUrl(QStringLiteral("qrc:/ui/main.qml")));
    if (engine.rootObjects().isEmpty())
        return 1;

    // 截图模式：取得 QML 根窗口，延时 1200 ms 后抓取窗口内容并退出。
    // 延时为初始布局和绘制留出时间，不代表渲染算法已经就绪。
    // 输出目录须预先存在；保存成功返回 0，窗口无效或保存失败返回 3。
    if (parser.isSet(QStringLiteral("capture"))) {
        const QString capturePath = parser.value(QStringLiteral("capture"));
        auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
        QTimer::singleShot(1200, &app, [window, capturePath, &app]() {
            const bool saved = window && window->grabWindow().save(capturePath);
            if (!saved)
                qCritical() << "Unable to capture window:" << capturePath;
            app.exit(saved ? 0 : 3);
        });
    }
    // 进入事件循环，处理用户输入、界面绘制和上述定时截图回调。
    // 窗口关闭或调用 app.exit() 后返回，记录退出结果并刷新日志。
    const int result = app.exec();
    Logger::info(QStringLiteral("Mini-Marmoset exited: %1").arg(result));
    Logger::flush();
    return result;
}
