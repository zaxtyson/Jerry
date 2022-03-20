//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_LOGGER_H
#define JERRY_LOGGER_H

#include <utils/NonCopyable.h>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include "Appender.h"
#include "AsyncFileAppender.h"

namespace jerry::logger {

enum class LogLevel : uint8_t { kDebug, kInfo, kWarn, kError, kFatal };

class Logger : NonCopyable {
  public:
    Logger() = default;
    ~Logger() = default;

    static void SetLogLevel(LogLevel level);
    static LogLevel GetLogLevel();
    static void SetAppender(Appender* appender);
    static void Log(LogLevel level, char* msg);

  private:
    inline static LogLevel global_level{LogLevel::kInfo};
    inline static std::unique_ptr<Appender> appender{new StderrAppender()};
};

const char* GetLogLevelString(LogLevel level);


#define __FILE_NAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define __LOGGER_LEVEL__(level) (GetLogLevelString(level))

#ifndef SHOW_LOG
#define LOG_BASE(level, fmt, ...) ((void)0);
#else
#define LOG_BASE(level, fmt, ...)                                   \
    if (level >= Logger::GetLogLevel()) {                           \
        char buf[1024];                                             \
        snprintf(buf,                                               \
                 sizeof(buf),                                       \
                 "0000-00-00 00:00:00.000000 %s %s:%d - " fmt "\n", \
                 __LOGGER_LEVEL__(level),                           \
                 __FILE_NAME__,                                     \
                 __LINE__,                                          \
                 ##__VA_ARGS__);                                    \
        Logger::Log(level, buf);                                    \
    }
#endif

#define LOG_DEBUG(fmt, ...) LOG_BASE(LogLevel::kDebug, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG_BASE(LogLevel::kInfo, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_BASE(LogLevel::kError, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG_BASE(LogLevel::kWarn, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) LOG_BASE(LogLevel::kFatal, fmt, ##__VA_ARGS__);

}  // namespace jerry::logger

using jerry::logger::Logger;
using jerry::logger::LogLevel;

#endif  // JERRY_LOGGER_H
