//
// Created by zaxtyson on 2021/9/19.
//

#include "EventLoop.h"
#include <sys/unistd.h>
#include <utils/FdHelper.h>
#include <net/TimerQueue.h>


EventLoop::EventLoop() : wakeupFd_(createEventFd()), poller_(new Poller), timerQueue_(new TimerQueue(this)) {
    wakeupChannel_.setFd(wakeupFd_);
    wakeupChannel_.enableReading();
    wakeupChannel_.setReadCallback([this](Date _) { handleWakeupEvent(); });
    poller_->addChannel(&wakeupChannel_);
}

void EventLoop::wakeup() const {
    // 向 eventfd 写数据, 触发可读事件
    LOG_DEBUG("Wakeup poller(fd=%d)", poller_->getFd());
    uint64_t on = 1;
    write(wakeupFd_, &on, sizeof(on));
}


void EventLoop::loop() {
    quit_ = false;
    while (!quit_) {
        // 处理 Epoll 事件
        activeChannelList_.clear();
        Date eventDate = poller_->poll(activeChannelList_);

        for (auto &channel: activeChannelList_) {
            channel->handleEvent(eventDate);
        }
        // 执行任务队列的任务
        executeTask();
    }
}

void EventLoop::executeTask() {
    TaskList todoTaskList;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        todoTaskList.swap(taskList_);
    }

    while (!todoTaskList.empty()) {
        LOG_DEBUG("Executing tasks in poller(fd=%d), remains task nums: %lu", getPollerFd(), todoTaskList.size());
        todoTaskList.front()();
        todoTaskList.pop();
    }
}

void EventLoop::handleWakeupEvent() const {
    // eventfd 可读, 把数据读走, 免得一直触发 EPOLLIN
    // 我们的目的只是唤醒 loop, 运行任务队列的 task
    uint64_t on = 1;
    read(wakeupFd_, &on, sizeof(on));
}

void EventLoop::runInLoop(Task &&task) {
    if (threadId_ == std::this_thread::get_id()) {
        // 如果该函数的调用着所在的线程和事件循环所在线程一致
        // 直接运行即可
        task();
        return;
    }

    // 否则就添加该任务到事件循环的任务队列
    {
        std::lock_guard<std::mutex> lock(mtx_);
        taskList_.emplace(task);
    }
    LOG_DEBUG("Add task to EventLoop, poller fd = %d", getPollerFd());
    // 跨线程唤醒当前事件循环去执行任务
    // eventfd 可读, loop 继续
    wakeup();
}

void EventLoop::assertInLoopThread() {
    auto tid = std::this_thread::get_id();
    if (threadId_ != tid) {
        LOG_FATAL("Assertion failed, caller not in EventLoop thread");
    }
}

EventLoop::~EventLoop() {

}

void EventLoop::cancel(TimerId timerId) {
    timerQueue_->cancelTimer(timerId);
}

TimerId EventLoop::runAt(Date when, Timer::Callback &&callback) {
    return timerQueue_->addTimer(when, std::move(callback));
}


TimerId EventLoop::runEvery(double interval, int repeatTimes, Timer::Callback &&callback) {
    return timerQueue_->addTimer(interval, repeatTimes, std::move(callback));
}

TimerId EventLoop::runEvery(double interval, Timer::Callback &&callback, Timer::StopCondition &&stopCondition) {
    return timerQueue_->addTimer(interval, std::move(callback), std::move(stopCondition));
}
