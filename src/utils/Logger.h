//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_LOGGER_H
#define JERRY_LOGGER_H

#include <utils/NonCopyable.h>
#include <string>

constexpr int BUFFER_SIZE = 1024;

enum class LogLevel {
    kDebug,
    kInfo,
    kWarn,
    kError,
    kFatal
};

/**
 * 简易日志类, 直接打印到标准输出
 */
class Logger : public NonCopyable {
public:
    static Logger &getInstance();

    void setLogLevel(LogLevel level);

    void log(std::string msg);

private:
    explicit Logger() = default;

    ~Logger() = default;

private:
    LogLevel level_;
};

#ifndef SHOW_LOG
#define LOG_BASE(level, fmt, ...) ((void)0)
#else
// 如果定义了 SHOW_LOG_DETAIL 宏, 则日志显示文件名/函数名/行号
#ifndef SHOW_LOG_DETAIL
#define LOG_BASE(level, fmt, ...)                               \
                do{                                                     \
                    Logger& logger = Logger::getInstance();             \
                    logger.setLogLevel(level);                          \
                    char buf[BUFFER_SIZE] = {0};                        \
                    snprintf(buf, BUFFER_SIZE, fmt, ##__VA_ARGS__);     \
                    logger.log(buf);                                    \
                } while (0)
#else
#define LOG_BASE(level, fmt, ...)                               \
                do{                                                     \
                    Logger& logger = Logger::getInstance();             \
                    logger.setLogLevel(level);                          \
                    char buf[BUFFER_SIZE] = {0};                        \
                    snprintf(buf, BUFFER_SIZE, "at %s:%d:%s | " fmt, __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__);     \
                    logger.log(buf);                                    \
                } while (0)
#endif  // SHOW_LOG_DETAIL
#endif  // NO_LOG

// 定义一些宏方便使用
#define LOG_DEBUG(fmt, ...)  LOG_BASE(LogLevel::kDebug, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  LOG_BASE(LogLevel::kInfo, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)  LOG_BASE(LogLevel::kError, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  LOG_BASE(LogLevel::kWarn, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...)  LOG_BASE(LogLevel::kFatal, fmt, ##__VA_ARGS__)

#endif //JERRY_LOGGER_H
