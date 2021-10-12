//
// Created by zaxtyson on 2021/10/11.
//

#ifndef JERRY_LOGGEROUTPUT_H
#define JERRY_LOGGEROUTPUT_H

#include <utils/NonCopyable.h>
#include <cstddef>

class LoggerOutput : NonCopyable {
public:
    LoggerOutput() = default;

    virtual ~LoggerOutput() = default;

    virtual void append(const char *msg, size_t len) = 0;

    virtual void stop() = 0;
};

class DefaultLoggerOutput : public LoggerOutput {
public:
    DefaultLoggerOutput() = default;

    ~DefaultLoggerOutput() override = default;

    void append(const char *msg, size_t len) override {
        printf("%s", msg);
    }

    void stop() override {

    }
};

#endif //JERRY_LOGGEROUTPUT_H
