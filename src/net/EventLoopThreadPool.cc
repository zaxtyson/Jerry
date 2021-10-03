//
// Created by zaxtyson on 2021/9/19.
//

#include <cassert>
#include <net/EventLoopThreadPool.h>

EventLoopThreadPool::EventLoopThreadPool(EventLoop *mainLoop, int workers) :
        mainLoop_(mainLoop), workers_(workers) {
    assert(workers >= 0);
    for (int i = 0; i < workers; i++) {
        threadList_.emplace_back(new EventLoopThread("IO-Thread/" + std::to_string(i)));
    }
}

EventLoopThreadPool::~EventLoopThreadPool() {
    for (auto &thread: threadList_) {
        // EventLoop 析构时会自动关闭线程, delete 即可
        delete thread;
    }
}

void EventLoopThreadPool::start() {
    if (workers_ > 0) {
        for (auto &thread: threadList_) {
            thread->start();
        }
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    if (workers_ == 0) {
        // 没有 worker 帮忙分担任务, 就全部自己来吧
        return mainLoop_;
    }

    EventLoop *loop = threadList_[index_]->getLoop();
    index_ = (index_ + 1) % workers_;  // workers 轮着来
    return loop;
}

void EventLoopThreadPool::stop() {
    for (auto &thread: threadList_) {
        thread->stop();
    }
}
