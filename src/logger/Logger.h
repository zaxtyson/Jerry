//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_LOGGER_H
#define JERRY_LOGGER_H

#include <utils/NonCopyable.h>
#include <utils/log/AsyncLogger.h>
#include <utils/log/LoggerOutput.h>
#include <cstdint>
#include <cstring>
#include <string>

namespace jerry::utils::logger {

constexpr const int kBufferSize = 1024;

enum class LogLevel : uint8_t { kDebug, kInfo, kWarn, kError, kFatal };

class Logger : NonCopyable {
  public:
    static Logger& GetInstance();
    static void SetLogLevel(LogLevel level);
    static void SetOutput(LoggerOutput* output);
    static void Log(LogLevel level, char* msg);

  private:
    inline static LogLevel global_log_level{LogLevel::kInfo};
    inline static LoggerOutput* output{new DefaultLoggerOutput()};
};

const char* GetLogLevelString(LogLevel level);

#define __FILE_NAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define __LOGGER_LEVEL__(level) (getLoggerLevelName(level))

#ifndef SHOW_LOG
#define LOG_BASE(level, fmt, ...) ((void)0)
#else
#define LOG_BASE(level, fmt, ...)                                   \
    do {                                                            \
        char buf[kBufferSize];                                      \
        snprintf(buf,                                               \
                 kBufferSize,                                       \
                 "0000-00-00 00:00:00.000000 %s %s:%d - " fmt "\n", \
                 __LOGGER_LEVEL__(level),                           \
                 __FILE_NAME__,                                     \
                 __LINE__,                                          \
                 ##__VA_ARGS__);                                    \
        Logger::GetInstance().Log(level, buf);                      \
    } while (0)
#endif


#define LOG_DEBUG(fmt, ...) LOG_BASE(LogLevel::kDebug, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG_BASE(LogLevel::kInfo, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_BASE(LogLevel::kError, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG_BASE(LogLevel::kWarn, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) LOG_BASE(LogLevel::kFatal, fmt, ##__VA_ARGS__)

}  // namespace jerry::utils::logger

#endif  // JERRY_LOGGER_H
