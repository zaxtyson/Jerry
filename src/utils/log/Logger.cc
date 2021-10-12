//
// Created by zaxtyson on 2021/9/19.
//

#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include "Logger.h"

LogLevel Logger::globalLogLevel_ = LogLevel::kInfo;
LoggerOutput *Logger::output_ = new DefaultLoggerOutput();

const char *getLoggerLevelName(LogLevel level) {
    static std::vector<std::string> mapping = {"[DEBUG]", "[INFO]", "[WARN]", "[ERROR]", "[FATAL]"};
    return mapping[static_cast<int>(level)].c_str();
}

void formatNowString(char *buffer) {
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


Logger &Logger::getInstance() {
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(LogLevel level) {
    globalLogLevel_ = level;
}

void Logger::log(LogLevel level, char *buf) {
    // 只打印级别高于或等于全局日志的级别的log
    if (static_cast<int>(level) < static_cast<int>(globalLogLevel_)) {
        return;
    }
    formatNowString(buf); // 填上时间
    output_->append(buf, strlen(buf));

    // FATAL 级别的错误直接终止程序
    if (level == LogLevel::kFatal) {
        char msg[128];
        snprintf(msg, sizeof(msg), "Terminate reason: %s\n", strerror(errno));
        output_->append(msg, strlen(msg));
        output_->stop();
        abort();
    }

}

void Logger::setOutput(LoggerOutput *output) {
    delete output_;
    output_ = output;
}



