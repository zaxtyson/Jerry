//
// Created by zaxtyson on 2021/10/8.
//

#ifndef JERRY_ASYNCLOGGER_H
#define JERRY_ASYNCLOGGER_H

#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <utils/CountDownLatch.h>
#include <utils/NonCopyable.h>
#include <utils/log/LoggerOutput.h>

#define LOG_BUFFER_SIZE 1024*1024*4

/**
 * 固定大小的 Buffer 类
 */
class Buffer : NonCopyable {
public:
    Buffer() = default;

    ~Buffer() = default;

    /**
     * 向 Buffer 追加一条日志
     * @param msg 字符串
     * @param len 字符串长度
     */
    void append(const char *msg, size_t len);

    /**
     * 获取 Buffer 可用字节数
     * @return
     */
    size_t available() const { return end() - cur_; }

    /**
     * 获取 Buffer 数据所占的字节数
     * @return
     */
    size_t size() const { return cur_ - data_; }

    /**
     * 获取 Buffer 起始位置的指针
     * @return
     */
    char *begin() { return data_; }

    /**
     * 清空 Buffer
     */
    void reset() { cur_ = data_; }

private:
    const char *end() const { return data_ + sizeof(data_); }

private:
    char data_[LOG_BUFFER_SIZE]{};
    char *cur_{data_};  // 指向当前写入位置
};

/**
 * 异步日志类, 负责日志缓冲和写磁盘
 */
class AsyncLogger : public LoggerOutput {
public :
    explicit AsyncLogger(const std::string &logPath, int autoFlushInterval = 3);

    ~AsyncLogger() override;

    /**
     * 设置日志自动刷新到磁盘的时间, 秒
     * @param autoFlushInterval
     */
    void setAutoFlushInterval(int autoFlushInterval);

    /**
     * 启动 flush 线程写入 Buffer 到磁盘
     */
    void start();

    /**
     * 停止 flush 线程, 刷新数据到磁盘
     */
    void stop() override;

public:
    /**
     * 写入一条日志
     * @param msg 日志数据
     * @param len 日志长度
     */
    void append(const char *msg, size_t len) override;


private:
    void flushThread();

private :
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferVector = std::vector<BufferPtr>;
private:
    int autoFlushInterval_;
    std::atomic<bool> running_{false};

    FILE *file_;  // 日志文件

    std::thread thread_{};
    CountDownLatch latch_{1};
    std::mutex mtx_{};
    std::condition_variable cond_{};

    BufferPtr curBuffer_{new Buffer};
    BufferPtr nextBuffer_{new Buffer};
    BufferVector fulledBuffers_{};
};


#endif //JERRY_ASYNCLOGGER_H
