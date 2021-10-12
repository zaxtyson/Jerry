//
// Created by zaxtyson on 2021/9/19.
//

#include "EventLoopThread.h"
#include <utils/log/Logger.h>

EventLoopThread::~EventLoopThread() {
    LOG_INFO("%s closing...", threadName_.c_str());
    loop_->quit();
    thread_.join();
}

EventLoop *EventLoopThread::getLoop() {
    return loop_;
}

void EventLoopThread::start() {
    thread_ = std::thread([this] {
        EventLoop loop;
        loop_ = &loop;
        threadId_ = std::this_thread::get_id();
        // 启动事件循环
        loop_->loop();
    });
}

EventLoopThread::EventLoopThread(const std::string &threadName) {
    threadName_ = threadName;
    LOG_INFO("Created %s", threadName_.c_str());
}

std::thread::id EventLoopThread::getThreadId() const {
    return threadId_;
}

void EventLoopThread::stop() {
    loop_->quit();
    thread_.join();
}
