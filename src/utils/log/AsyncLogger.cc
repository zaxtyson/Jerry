//
// Created by zaxtyson on 2021/10/8.
//

#include "AsyncLogger.h"
#include <cstring>
#include <memory>
#include <cassert>
#include <chrono>
#include <unistd.h>
#include <utils/Date.h>


void Buffer::append(const char *msg, size_t len) {
    if (available() >= len) {
        memcpy(cur_, msg, len);
        cur_ += len;
    }
}

AsyncLogger::AsyncLogger(const std::string &logPath, int autoFlushInterval) :
        autoFlushInterval_(autoFlushInterval){
    file_ = fopen(logPath.c_str(), "a+");
    assert(file_ != nullptr);
}

AsyncLogger::~AsyncLogger() {
    stop();
}

void AsyncLogger::append(const char *msg, size_t len) {
    std::lock_guard<std::mutex> lock(mtx_);
    // 当前的 Buffer 还能用
    if (curBuffer_->available() >= len) {
        curBuffer_->append(msg, len);
        return;
    }

    // 当前的 Buffer 写不下了, 切换备用 Buffer
    fulledBuffers_.push_back(std::move(curBuffer_));

    // 如果日志产生的非常非常快, 而此时还在写磁盘, 两个 Buffer 在写磁盘时被写满了
    // 就增加 Buffer 数量, 这种情况应该很少遇到, 格式化字符串还需要时间
    if (!nextBuffer_) {
        nextBuffer_ = std::make_unique<Buffer>();
    }

    curBuffer_ = std::move(nextBuffer_);
    curBuffer_->append(msg, len);

    // 唤醒 flush 线程写数据
    // 此时可能在写磁盘. 并没有在 wait, notify 无效
    cond_.notify_one();
}

void AsyncLogger::flushThread() {
    BufferPtr tmpBuffer1 = std::make_unique<Buffer>();
    BufferPtr tmpBuffer2 = std::make_unique<Buffer>();
    BufferVector buffersToWrite;

    latch_.countDown(); // flush 线程启动好了, 才唤醒主线程

    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mtx_);

            // 如果产生日志的速度有点慢, 没有 Buffer 被写满
            if (fulledBuffers_.empty()) {
                // 先等 3s, 当然也可能提前被唤醒
                if (cond_.wait_for(lock, std::chrono::seconds(autoFlushInterval_)) == std::cv_status::timeout) {
//                    printf("timeout, force flush\n");
                }
            }

            // 超时或者被提前唤醒, 不管当前的 Buffer 有没有满, 也加入刷磁盘队列
            fulledBuffers_.push_back(std::move(curBuffer_));
            // 换两个临时的 Buffer 顶上去
            curBuffer_ = std::move(tmpBuffer1);
            if (!nextBuffer_) {
                nextBuffer_ = std::move(tmpBuffer2);
            }

            // 切换队列, 使得写磁盘的时候不需要锁
            std::swap(fulledBuffers_, buffersToWrite);
        }

        // 写磁盘的时候不需要锁, 这里可能要花点时间
        // muduo 遇到大量日志冲击的情况, 会丢掉日志, 保留 25 个 Buffer
//       printf("flush to disk, %zu buffers\n", buffersToWrite.size());
        for (auto &&buffer: buffersToWrite) {
            fwrite(buffer->begin(), buffer->size(), 1, file_);
            fflush(file_);
        }

//        if(syncToDisk_){
//             fdatasync(fileno(file_)); // 立刻同步数据到磁盘
//        }

        // 只留 2 个 Buffer
        if (buffersToWrite.size() > 2) {
            buffersToWrite.resize(2);
        }

        // 这两个 Buffer 拿来复用, 免得重复创建
        if (!tmpBuffer1) {
            tmpBuffer1 = std::move(buffersToWrite[0]);
            tmpBuffer1->reset();
        }
        if (!tmpBuffer2) {
            tmpBuffer2 = std::move(buffersToWrite[1]);
            tmpBuffer2->reset();
        }

        buffersToWrite.clear();

    }
}

void AsyncLogger::start() {
    running_ = true;
    thread_ = std::thread([this]() { flushThread(); });
    latch_.wait(); // 主线程先打住, 等 flush 线程准备完成
}

void AsyncLogger::stop() {
    if (!running_) return;
    cond_.notify_one();  // 如果还在等待, 立刻唤醒
    running_ = false; // 等数据写完再停止
    thread_.join();
    fsync(fileno(file_)); // 刷新脏页到磁盘
    fclose(file_);
}

void AsyncLogger::setAutoFlushInterval(int autoFlushInterval) {
    autoFlushInterval_ = autoFlushInterval;
}
