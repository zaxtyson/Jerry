//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_EVENTLOOPTHREAD_H
#define JERRY_EVENTLOOPTHREAD_H

#include <string>
#include <thread>
#include <net/EventLoop.h>
#include <utils/NonCopyable.h>


class EventLoopThread : NonCopyable {
public:
    EventLoopThread() = default;

    /**
     * 创建事件循环线程
     * @param threadName 线程名
     */
    explicit EventLoopThread(const std::string &threadName);

    ~EventLoopThread();

    /**
     * 获取当前线程中运行的事件循环
     * @return
     */
    EventLoop *getLoop();

    /**
     * 启动线程, 创建一个事件循环
     */
    void start();

    /**
     * 停止事件循环
     */
    void stop();

    /**
     * 获取线程 ID
     * @return
     */
    std::thread::id getThreadId() const;

private:
    std::thread thread_{};
    std::thread::id threadId_{0};
    std::string threadName_{"IO-Thread"};
    EventLoop *loop_{nullptr};
};


#endif //JERRY_EVENTLOOPTHREAD_H
