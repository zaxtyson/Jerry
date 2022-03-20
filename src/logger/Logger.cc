//
// Created by zaxtyson on 2021/9/19.
//

#include "Logger.h"
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <map>

namespace jerry::utils::logger {

const char* GetLogLevelString(LogLevel level) {
    static const std::map<LogLevel, const char*> level_map = {{LogLevel::kDebug, "DEBUG"},
                                                              {LogLevel::kInfo, "INFO"},
                                                              {LogLevel::kWarn, "WARN"},
                                                              {LogLevel::kError, "ERROR"},
                                                              {LogLevel::kFatal, "FATAL"}};
    return level_map.at(level);
}

void FormatNowString(char* buffer) {
    // 将 buffer 前面 26 个字符填充为当前时间
    // 年月日时分秒缓存, 毫秒才格式化
    static time_t lastSecond = 0;
    static char prefixOfTime[21] = {0};
    static timeval tv{};

    gettimeofday(&tv, nullptr);
    // 秒数改变时才格式化一次
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
    buffer[26] = ' ';  // 去掉第 27 个字符 '\0'
}


Logger& Logger::GetInstance() {
    static Logger logger;
    return logger;
}

void Logger::SetLogLevel(LogLevel level) {
    global_log_level = level;
}

void Logger::Log(LogLevel level, char* msg) {
    // 只打印级别高于或等于全局日志的级别的log
    if (level < global_log_level) {
        return;
    }
    FormatNowString(msg);  // 填上时间
    output->append(msg, strlen(msg));

    // FATAL 级别的错误直接终止程序
    if (level == LogLevel::kFatal) {
        char err_msg[128];
        snprintf(err_msg, sizeof(err_msg), "Terminate reason: %s\n", strerror(errno));
        output->append(err_msg, strlen(err_msg));
        output->stop();
        abort();
    }
}

void Logger::SetOutput(LoggerOutput* output) {
    delete output;
    output = output;
}

}  // namespace jerry::utils::logger
