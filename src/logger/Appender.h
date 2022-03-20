//
// Created by zaxtyson on 2021/10/11.
//

#ifndef JERRY_APPENDER_H
#define JERRY_APPENDER_H

#include <iostream>
#include "utils/NonCopyable.h"

namespace jerry::logger {

class Appender : NonCopyable {
  public:
    Appender() = default;
    virtual ~Appender() = default;

    virtual void Append(const char* msg, size_t len) = 0;
    virtual void Flush(){};
};

class StderrAppender : public Appender {
  public:
    void Append(const char* msg, [[maybe_unused]] size_t len) override { std::cerr << msg; }
};

}  // namespace jerry::logger

#endif  // JERRY_APPENDER_H
