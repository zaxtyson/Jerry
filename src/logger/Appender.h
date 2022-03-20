//
// Created by zaxtyson on 2021/10/11.
//

#ifndef JERRY_LOGGEROUTPUT_H
#define JERRY_LOGGEROUTPUT_H

#include <cstddef>
#include "utils/NonCopyable.h"

namespace jerry::logger {

class LoggerOutput : NonCopyable {
  public:
    LoggerOutput() = default;
    virtual ~LoggerOutput() = default;

    virtual void Append(const char* msg, size_t len) = 0;
    virtual void Stop() = 0;
};

class ConsoleLoggerOutput : LoggerOutput {
  public:
    ConsoleLoggerOutput() = default;
    ~ConsoleLoggerOutput() override = default;

    void Append(const char* msg, size_t len) override { printf("%s", msg); }
    void Stop() override {}
};

}  // namespace jerry::logger

#endif  // JERRY_LOGGEROUTPUT_H
