//
// Created by zaxtyson on 2021/10/8.
//

#ifndef JERRY_ASYNCFILEAPPENDER_H
#define JERRY_ASYNCFILEAPPENDER_H

#include <atomic>
#include <condition_variable>
#include <fstream>  // ofstream
#include <memory>
#include <string_view>
#include <thread>
#include <vector>
#include "Appender.h"
#include "utils/CountDownLatch.h"
#include "utils/Mutex.h"
#include "utils/NonCopyable.h"

namespace jerry::logger {

constexpr const int kFixedBufferSize = 1024 * 1024 * 4;

class FixedBuffer : NonCopyable {
  public:
    FixedBuffer() = default;
    ~FixedBuffer() = default;

    void Append(const char* msg, size_t len);
    size_t AvailableBytes() const;
    size_t ReadableBytes() const;
    void Reset();
    const char* Begin();
    const char* End();
    const char* Begin() const;
    const char* End() const;

  private:
    char data[kFixedBufferSize]{};
    char* cur{data};
};


class AsyncFileAppender : public Appender {
  public:
    explicit AsyncFileAppender(std::string_view path, int auto_flush_interval = 3);
    ~AsyncFileAppender() override = default;

    void Start();
    void Stop();
    void Flush() override;
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
    std::condition_variable cond GUARDED_BY(mtx);

    BufferPtr cur_buffer PT_GUARDED_BY(mtx);
    BufferPtr next_buffer PT_GUARDED_BY(mtx);
    BufferVector fulled_buffers GUARDED_BY(mtx);
};

}  // namespace jerry::logger

#endif
