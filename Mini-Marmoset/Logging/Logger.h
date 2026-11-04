#pragma once

#include <QString>
#include <QtGlobal>
#include <memory>

namespace spdlog { class logger; }

// 统一封装文件日志，并将 Qt/QML 消息转交给 spdlog；同步写入由线程安全 sink 负责。
class Logger
{
public:
    // Call once on the main thread before starting workers. An empty directory
    // uses the executable directory's logs/ folder.
    // 启动时在主线程调用；空目录表示可执行文件旁的 logs/。
    // 初始化成功后再次调用保持原配置，失败返回 false，允许后续重试。
    static bool initialize(const QString &directory = QString());
    // 分级记录 UTF-8 文本；QString 可用 arg() 组合参数，不直接解释消息中的格式占位符。
    static void debug(const QString &message);
    static void info(const QString &message);
    static void warning(const QString &message);
    static void error(const QString &message);
    // 主动刷新缓冲；程序正常退出时也会刷新。
    static void flush();

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;
    // 函数局部静态对象管理单例生命周期，禁止外部构造和复制。
    static Logger &instance();
    // Qt 消息回调：补充分类、源码位置，再按 Qt 严重级别写入。
    static void handleQtMessage(QtMsgType type, const QMessageLogContext &context,
                                const QString &message);
    // 所有日志接口的共同写入路径；未初始化时回退至标准错误输出。
    static void write(QtMsgType type, const QString &message);

    // 保存文件 logger 和安装前的 Qt 处理器，以便退出时恢复。
    std::shared_ptr<spdlog::logger> logger_;
    QtMessageHandler previousHandler_ = nullptr;
};
