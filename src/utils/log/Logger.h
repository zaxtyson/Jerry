//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_LOGGER_H
#define JERRY_LOGGER_H

#include <utils/NonCopyable.h>
#include <string>
#include <utils/log/AsyncLogger.h>
#include <utils/log/LoggerOutput.h>
#include <string.h>

constexpr int BUFFER_SIZE = 1024;

enum class LogLevel : int {
    kDebug = 0,
    kInfo,
    kWarn,
    kError,
    kFatal
};

/**
 * 简易日志类
 */
class Logger : public NonCopyable {
public:
    /**
     * 获取全局单例对象
     * @return
     */
    static Logger &getInstance();

    /**
     * 设置全局日志级别
     * @param level
     */
    static void setLogLevel(LogLevel level);

    /**
     * 设置日志处理器, 把日志持久化
     * 
     * @param output 
     */
    static void setOutput(LoggerOutput *output);

    /**
     * 记录一条日志, 只记录高于全局日志级别的log
     * @param level
     * @param msg
     */
    static void log(LogLevel level, char *msg);

private:
    explicit Logger() = default;


private:
    static LogLevel globalLogLevel_;
    static LoggerOutput *output_;
};

const char *getLoggerLevelName(LogLevel level);

#define __FILE_NAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define __LOGGER_LEVEL__(level) (getLoggerLevelName(level))

#ifndef SHOW_LOG
#define LOG_BASE(level, fmt, ...) ((void)0)
#else
#define LOG_BASE(level, fmt, ...)                               \
                do{                                              \
                    char buf[BUFFER_SIZE];                       \
                    snprintf(buf, BUFFER_SIZE, "0000-00-00 00:00:00.000000 %s %s:%d - " fmt "\n", __LOGGER_LEVEL__(level), __FILE_NAME__, __LINE__, ##__VA_ARGS__);     \
                    Logger::getInstance().log(level, buf);                                    \
                } while (0)
#endif

// 定义一些宏方便使用
#define LOG_DEBUG(fmt, ...)  LOG_BASE(LogLevel::kDebug, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  LOG_BASE(LogLevel::kInfo, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)  LOG_BASE(LogLevel::kError, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  LOG_BASE(LogLevel::kWarn, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...)  LOG_BASE(LogLevel::kFatal, fmt, ##__VA_ARGS__)

#endif //JERRY_LOGGER_H
