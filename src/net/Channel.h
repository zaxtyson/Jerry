//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_CHANNEL_H
#define JERRY_CHANNEL_H

#include <utils/NonCopyable.h>
#include <utils/Date.h>
#include <functional>
#include <cstdint>
#include <memory>
#include <unistd.h>

/**
 * Channel 类用于封装类 Epoll 的事件,
 * 在事件循环中, Poller 返回活跃的 Channels,
 * 然后调用其 handleEvent() 方法
 * 该方法根据具体的事件类型调用绑定的回调函数
 * Channel 析构时自动关闭 fd
 */
class Channel : NonCopyable {
public:
    using ReadCallback = std::function<void(Date)>; // 我们对数据到来的时间也感兴趣
    using Callback = std::function<void()>;
public:
    Channel() : fd_(-1) {}

    explicit Channel(int fd) : fd_(fd) {}

    ~Channel();

    /**
     * 获取 Channel 管理的 fd
     * @return fd
     */
    int getFd() const { return fd_; }

    /**
     * 设置 Channel 管理的 fd
     * @param fd
     */
    void setFd(int fd) { fd_ = fd; }

    /**
     * 获取 fd 关注的事件
     * @return epoll events
     */
    uint32_t getFocusedEvent() const { return events_; }

    bool isNoneEvent() const { return events_ == kNoneEvent; }

    void disableAllEvent() { events_ = kNoneEvent; }

    bool isReadable() const { return events_ == kReadEvent; }

    void enableReading() { events_ |= kReadEvent; }

    void disableReading() { events_ &= ~kReadEvent; }

    /**
     * 是否关注了可写事件
     * @return
     */
    bool isWritable() const { return events_ & kWriteEvent; }

    void enableWriting() { events_ |= kWriteEvent; }

    void disableWriting() { events_ &= ~kWriteEvent; }

    /**
     * 设置真正发生的事件, 由 poller 负责填写
     * @param realEvents
     */
    void setRealEvents(uint32_t realEvents) {
        revents_ = realEvents;
    }

    /**
     * 有事件触发时, 调用上面注册的回调函数处理对应的事件
     * @param date 事件发生的时间
     */
    void handleEvent(Date date);

    /**
     * 设置可读事件的回调函数
     * @param callback
     */
    void setReadCallback(const ReadCallback &callback) { readCallback_ = callback; }

    /**
     * 设置可写事件的回调函数
     * @param callback
     */
    void setWriteCallback(const Callback &callback) { writeCallback_ = callback; }

    /**
     * 设置错误事件的回调函数
     * @param callback
     */
    void setErrorCallback(const Callback &callback) { errorCallback_ = callback; }

    /**
     * 设置连接关闭事件的回调函数
     * @param callback
     */
    void setCloseCallback(const Callback &callback) { closeCallback_ = callback; }

private:
    int fd_;  // Channel 管理的 fd
    uint32_t events_{0}; // 我们关注的事件
    uint32_t revents_{0};  // 实际发生的事件, 由 poller 设置其值
private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
private:
    ReadCallback readCallback_;
    Callback writeCallback_;
    Callback closeCallback_;
    Callback errorCallback_;
};


#endif //JERRY_CHANNEL_H
