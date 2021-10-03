//
// Created by zaxtyson on 2021/9/19.
//

#include "Logger.h"
#include <iostream>
#include <utils/Date.h>

Logger &Logger::getInstance() {
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(LogLevel level) {
    level_ = level;
}

void Logger::log(std::string msg) {
    std::cout << Date::now().toString() << " ";
    switch (level_) {
        case LogLevel::kDebug:
            std::cout << "[DEBUG]";
            break;
        case LogLevel::kInfo:
            std::cout << "[INFO]";
            break;
        case LogLevel::kError:
            std::cout << "[ERROR]";
            break;
        case LogLevel::kWarn:
            std::cout << "[WARN]";
            break;
        case LogLevel::kFatal:
            std::cout << "[FATAL]";
    }

    std::cout << " " << msg << std::endl;
}


