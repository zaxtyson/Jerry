//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_EVENTLOOPTHREADPOOL_H
#define JERRY_EVENTLOOPTHREADPOOL_H

#include <utils/NonCopyable.h>
#include <net/EventLoopThread.h>
#include <net/EventLoop.h>
#include <iostream>
#include <vector>
#include <string>

class EventLoopThreadPool : NonCopyable {
public:
    /**
     * 创建事件循环线程池
     * @param mainLoop 用户在程序主线程创建的事件循环
     * @param workers 需创建的线程数量
     */
    explicit EventLoopThreadPool(EventLoop *mainLoop, int workers);

    ~EventLoopThreadPool();

    /**
     * 启动线程池中所有的 Worker 线程
     */
    void start();

    /**
     * 从事件循环线程池获取一个事件循环线程,
     * 如果没有启用线程池, 则返回当前事件循环
     * 用于分配 Channel, 实现负载均衡
     * @return
     */
    EventLoop *getNextLoop();

    /**
     * 停止所有线程(等待其完成任务)
     */
    void stop();

private:
    EventLoop *mainLoop_{nullptr};
    int workers_{0};  // 事件循环池的线程数量
    std::vector<EventLoopThread *> threadList_{};
    int index_{0};  // 下次分配任务给这个线程
};


#endif //JERRY_EVENTLOOPTHREADPOOL_H
