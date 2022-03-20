//
// Created by zaxtyson on 2021/10/8.
//

#ifndef JERRY_ASYNCLOGGER_H
#define JERRY_ASYNCLOGGER_H

#include <atomic>
#include <condition_variable>
#include <fstream>  // ofstream
#include <memory>
#include <mutex>
#include <string_view>
#include <thread>
#include <vector>
#include "Appender.h"
#include "utils/CountDownLatch.h"
#include "utils/NonCopyable.h"

namespace jerry::logger {

constexpr const int kFixedBufferSize = 1024 * 1024 * 4;

class FixedBuffer : NonCopyable {
  public:
    FixedBuffer() = default;
    ~FixedBuffer() = default;

    void Append(const char* msg, size_t len);
    size_t AvailableBytes() const { return End() - cur; }
    size_t ReadableBytes() const { return cur - data; }
    char* Begin() { return data; }
    void Reset() { cur = data; }

  private:
    const char* End() const { return data + sizeof(data); }

  private:
    char data[kFixedBufferSize]{};
    char* cur{data};
};


class AsyncLogger : Appender {
  public:
    explicit AsyncLogger(std::string_view path, int auto_flush_interval = 3);
    ~AsyncLogger() override;

    void Start();
    void Stop() override;
    void Append(const char* msg, size_t len) override;

  private:
    void FlushThread();

  private:
    using BufferPtr = std::unique_ptr<FixedBuffer>;
    using BufferVector = std::vector<BufferPtr>;

  private:
    int auto_flush_interval;
    std::atomic<bool> stop{false};
    std::ofstream log_file;
    std::thread th{};
    utils::CountDownLatch latch{1};
    std::mutex mtx{};
    std::condition_variable cond{};

    BufferPtr cur_buffer{new FixedBuffer()};
    BufferPtr next_buffer{new FixedBuffer()};
    BufferVector fulled_buffers{};
};

}  // namespace jerry::logger

#endif
