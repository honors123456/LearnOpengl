#include "Logger.h"

#include <QDir>
#include <QCoreApplication>
#include <spdlog/logger.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <cstdio>

// 首次使用时构造单例，进程正常退出时由静态对象析构完成收尾。
Logger &Logger::instance()
{
    static Logger logger;
    return logger;
}

// 创建日志目录和滚动文件 sink；仅在全部配置成功后接管 Qt 消息。
bool Logger::initialize(const QString &directory)
{
    auto &self = instance();
    // 避免重复初始化时替换当前文件或重复安装消息处理器。
    if (self.logger_)
        return true;

    // 默认以可执行文件位置定位 logs/，不依赖启动时的工作目录。
    QString logDirectory = directory;
    if (logDirectory.isEmpty()) {
        logDirectory = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("logs"));
    }
    if (!QDir().mkpath(logDirectory))
        return false;

    try {
        // 固定主日志文件名；Windows 使用宽字符文件名以支持中文路径。
        const QString path = QDir(logDirectory).filePath(QStringLiteral("Mini-Marmoset.log"));
#ifdef Q_OS_WIN
        const auto filename = path.toStdWString();
#else
        const auto filename = path.toUtf8().toStdString();
#endif
        // 每个文件最大 5 MB，保留 3 个滚动备份；_mt sink 对并发写入加锁。
        auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            filename, 5 * 1024 * 1024, 3);
        self.logger_ = std::make_shared<spdlog::logger>("Mini-Marmoset", sink);
        // 日志包含本地时间、级别和线程 ID；info 及以上每条刷新以便及时排查。
        self.logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [thread %t] %v");
        self.logger_->set_level(spdlog::level::debug);
        self.logger_->flush_on(spdlog::level::info);
        // 保留原处理器供转发和退出恢复；安装之后的 Qt/QML 消息进入此日志。
        self.previousHandler_ = qInstallMessageHandler(&Logger::handleQtMessage);
        info(QStringLiteral("Logger initialized: %1").arg(path));
        return true;
    } catch (const std::exception &e) {
        // 初始化异常时释放未完成配置的 logger，使用 stderr 报告，避免再次调用 Qt 日志。
        self.logger_.reset();
        std::fprintf(stderr, "Logger initialization failed: %s\n", e.what());
        return false;
    }
}

// 先恢复原 Qt 消息处理器，再刷新文件，结束本类对消息的接管。
Logger::~Logger()
{
    if (logger_) {
        qInstallMessageHandler(previousHandler_);
        logger_->flush();
    }
}

// QString 转成 UTF-8 后统一写入；消息作为参数传递，花括号会按原文保留。
void Logger::write(QtMsgType type, const QString &message)
{
    const auto bytes = message.toUtf8();
    const auto &logger = instance().logger_;
    if (!logger) {
        std::fprintf(stderr, "%s\n", bytes.constData());
        return;
    }
    // 将 Qt 的 debug/info/warning/critical/fatal 映射到 spdlog 对应级别。
    auto level = spdlog::level::info;
    switch (type) {
    case QtDebugMsg: level = spdlog::level::debug; break;
    case QtWarningMsg: level = spdlog::level::warn; break;
    case QtCriticalMsg: level = spdlog::level::err; break;
    case QtFatalMsg: level = spdlog::level::critical; break;
    case QtInfoMsg: break;
    }
    logger->log(level, "{}", bytes.constData());
}

// 保留有意义的分类和源码行号，方便从 QML/C++ 报错定位到调用位置。
void Logger::handleQtMessage(QtMsgType type, const QMessageLogContext &context,
                             const QString &message)
{
    QString text = message;
    if (context.category && QString::fromLatin1(context.category) != QStringLiteral("default"))
        text.prepend(QStringLiteral("[%1] ").arg(QString::fromLatin1(context.category)));
    if (context.file)
        text += QStringLiteral(" (%1:%2)").arg(QString::fromUtf8(context.file)).arg(context.line);
    write(type, text);
    // Keep Qt's existing console/debugger output when a custom handler exists.
    if (instance().previousHandler_)
        instance().previousHandler_(type, context, message);
}

// 对外提供简洁的分级入口，共用 write()，避免各模块重复处理日志格式。
void Logger::debug(const QString &message) { write(QtDebugMsg, message); }
void Logger::info(const QString &message) { write(QtInfoMsg, message); }
void Logger::warning(const QString &message) { write(QtWarningMsg, message); }
void Logger::error(const QString &message) { write(QtCriticalMsg, message); }
void Logger::flush()
{
    if (instance().logger_)
        instance().logger_->flush();
}
