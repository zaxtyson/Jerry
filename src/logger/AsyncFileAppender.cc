//
// Created by zaxtyson on 2021/10/8.
//

#include "AsyncFileAppender.h"
#include <unistd.h>
#include <cassert>
#include <chrono>
#include <cstring>
#include <memory>
#include "utils/DateTime.h"

namespace jerry::logger {

void FixedBuffer::Append(const char* msg, size_t len) {
    if (AvailableBytes() >= len) {
        memcpy(cur, msg, len);
        cur += len;
    }
}

size_t FixedBuffer::AvailableBytes() const {
    return End() - cur;
}

size_t FixedBuffer::ReadableBytes() const {
    return cur - data;
}

const char* FixedBuffer::Begin() {
    return data;
}

const char* FixedBuffer::Begin() const {
    return data;
}

const char* FixedBuffer::End() const {
    return data + sizeof(data);
}

const char* FixedBuffer::End() {
    return data + sizeof(data);
}

void FixedBuffer::Reset() {
    cur = data;
}

AsyncFileAppender::AsyncFileAppender(std::string_view path, int auto_flush_interval)
    : auto_flush_interval{auto_flush_interval},
      cur_buffer{new FixedBuffer()},
      next_buffer{new FixedBuffer()} {
    log_file.rdbuf()->pubsetbuf(nullptr, 0);
    log_file.open(path.data(), std::ofstream::out | std::ofstream::app);
    assert(log_file.good());
}

void AsyncFileAppender::Append(const char* msg, size_t len) {
    LockGuard lock(mtx);

    if (cur_buffer->AvailableBytes() >= len) {
        cur_buffer->Append(msg, len);
        return;
    }

    // current buffer is full, switch to next buffer
    fulled_buffers.emplace_back(std::move(cur_buffer));

    if (!next_buffer) {
        next_buffer = std::make_unique<FixedBuffer>();
    }

    cur_buffer = std::move(next_buffer);
    cur_buffer->Append(msg, len);

    Flush();
}

void AsyncFileAppender::FlushThread() {
    BufferPtr tmp_buffer1 = std::make_unique<FixedBuffer>();
    BufferPtr tmp_Buffer2 = std::make_unique<FixedBuffer>();
    BufferVector buffers_to_write;

    // wake up main thread after flush thread started
    latch.CountDown();

    while (!stop.load()) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (fulled_buffers.empty()) {
                cond.wait_for(lock, std::chrono::seconds(auto_flush_interval));
            }

            // some buffer fulled or timeout
            fulled_buffers.push_back(std::move(cur_buffer));
            cur_buffer = std::move(tmp_buffer1);

            if (!next_buffer) {
                next_buffer = std::move(tmp_Buffer2);
            }

            std::swap(fulled_buffers, buffers_to_write);
        }

        // write fulled buffer to disk
        for (auto&& buffer : buffers_to_write) {
            log_file.write(buffer->Begin(), buffer->ReadableBytes());
        }

        // if too many buffer are created, keep only 2
        if (buffers_to_write.size() > 2) {
            buffers_to_write.resize(2);
        }

        // reuse the tmp buffer
        if (!tmp_buffer1) {
            tmp_buffer1 = std::move(buffers_to_write[0]);
            tmp_buffer1->Reset();
        }

        if (!tmp_Buffer2) {
            tmp_Buffer2 = std::move(buffers_to_write[1]);
            tmp_Buffer2->Reset();
        }

        buffers_to_write.clear();
    }
}

void AsyncFileAppender::Start() {
    th = std::thread([this]() { FlushThread(); });
    // block caller thread until flush thread start
    latch.Wait();
}

void AsyncFileAppender::Stop() {
    // wait flush to finished
    Flush();
    stop.store(true);
    th.join();
}

void AsyncFileAppender::Flush() {
    // wake up flush thread and flush unfilled buffer right now
    cond.notify_one();
}

}  // namespace jerry::logger
