//
// Created by zaxtyson on 2021/9/19.
//

#include "Logger.h"
#include <sys/time.h>
#include <cerrno>
#include <cstring>
#include <map>

namespace jerry::logger {

const char* GetLogLevelString(LogLevel level) {
    static const std::map<LogLevel, const char*> level_map = {{LogLevel::kDebug, "[DEBUG]"},
                                                              {LogLevel::kInfo, "[INFO]"},
                                                              {LogLevel::kWarn, "[WARN]"},
                                                              {LogLevel::kError, "[ERROR]"},
                                                              {LogLevel::kFatal, "[FATAL]"}};
    return level_map.at(level);
}

void FormatNowString(char* buffer) {
    // only format milliseconds
    static time_t lastSecond = 0;
    static char prefixOfTime[21] = {0};
    static timeval tv{};

    gettimeofday(&tv, nullptr);
    // format when seconds changed
    if (lastSecond != tv.tv_sec) {
        lastSecond = tv.tv_sec;
        tm tm_time{};
        gmtime_r(&tv.tv_sec, &tm_time);
        snprintf(prefixOfTime,
                 sizeof(prefixOfTime),
                 "%4d-%02d-%02d %02d:%02d:%02d.",  // len = 20
                 tm_time.tm_year + 1900,
                 tm_time.tm_mon + 1,
                 tm_time.tm_mday,
                 tm_time.tm_hour + 8,
                 tm_time.tm_min,
                 tm_time.tm_sec);
    }
    // "2021-00-00 00:00:00.000000" len = 26
    snprintf(buffer, 27, "%s%06ld", prefixOfTime, tv.tv_usec / 1000);
    // buffer = "2021-00-00 00:00:00.000000" "\0" "other str"
    buffer[26] = ' ';  // remove '\0'
}

void Logger::SetLogLevel(LogLevel level) {
    global_level = level;
}

void Logger::Log(LogLevel level, char* msg) {
    if (level < global_level) {
        return;
    }

    FormatNowString(msg);
    appender->Append(msg, strlen(msg));

    if (level == LogLevel::kFatal) {
        char err_msg[128];
        snprintf(err_msg, sizeof(err_msg), "Terminate reason: %s\n", strerror(errno));
        appender->Append(err_msg, strlen(err_msg));
        appender->Flush();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // wait for async flush
        abort();
    }
}

void Logger::SetAppender(Appender* appender) {
    Logger::appender.reset(appender);
}

LogLevel Logger::GetLogLevel() {
    return Logger::global_level;
}

}  // namespace jerry::logger
