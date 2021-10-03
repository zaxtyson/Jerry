//
// Created by zaxtyson on 2021/10/2.
//

#include <net/TimerQueue.h>
#include <utils/FdHelper.h>
#include <cassert>
#include <utils/Logger.h>
#include <net/EventLoop.h>

TimerQueue::TimerQueue(EventLoop *loop) :
        loop_(loop), timerFd_(createTimerFd()) {

    timerFdChannel_.setFd(timerFd_);
    timerFdChannel_.enableReading();
    timerFdChannel_.setReadCallback([this](Date _) { handleTimeoutEvent(); });
    loop->addChannel(&timerFdChannel_);
}


void TimerQueue::addTimerInLoop(Timer *newTimer) {
    loop_->assertInLoopThread();
    LOG_DEBUG("Add Timer(id=%lu), timeout at %s", newTimer->getTimerId(),
              newTimer->getExpiration().toString().c_str());
    timerIdMap_[newTimer->getTimerId()] = newTimer;

    // 还没有定时器, 或者这个定时器的过期时间比最小堆的都早,
    // 则下一次 timerfd 触发时间就设定为这个定时器的过期时间
    if (timers_.empty() || newTimer->getExpiration() < getNearestTimerExpiration()) {
        LOG_DEBUG("Update timerfd(fd=%d) in poller(fd=%d), set next timeout at %s", timerFd_, loop_->getPollerFd(),
                  newTimer->getExpiration().toString().c_str());
        // 在 Loop 中添加定时器可能要排队, 因此加入时可能已经超时
        auto us = newTimer->getExpiration() - Date::now();
        us = (us > 0) ? us : 0;  // 超时的立刻触发
        setTimerFd(timerFd_, us);
    }

    timers_.push(newTimer);
}

void TimerQueue::cancelTimer(TimerId timerId) {
    loop_->runInLoop([this, timerId]() { this->cancelInLoop(timerId); });
}

void TimerQueue::cancelInLoop(TimerId timerId) {
    loop_->assertInLoopThread();
    if (timerIdMap_.find(timerId) != timerIdMap_.end()) {
        LOG_DEBUG("Timer(id=%lu) has been cancelled", timerIdMap_[timerId]->getTimerId());
        timerIdMap_[timerId]->cancel();  // 取消该计时器, 先不删除
        timerIdMap_.erase(timerId); // 不再记录它
    }
}

void TimerQueue::handleTimeoutEvent() {
    loop_->assertInLoopThread();
    // 取走 timerfd 的数据
    uint64_t n;
    read(timerFd_, &n, sizeof(n));

    // 获取超时的计时器
    auto expiredTimers = popExpiredTimers();
    LOG_DEBUG("Poller(fd=%d) get expired Timers: %zu", loop_->getPollerFd(), expiredTimers.size());

    // 设置 timerfd 下一次触发时间为未超时的定时器中最早触发的那个
    if (!timers_.empty()) {
        int64_t nextTimeout = getNearestTimerExpiration() - Date::now();
        nextTimeout = (nextTimeout > 0) ? nextTimeout : 0;
        setTimerFd(timerFd_, nextTimeout);
    }

    for (auto &timer: expiredTimers) {
        timer->run(); // 执行定时器回调

        // 更新定时器状态, 如果可以再次触发, 则加入到定时器队列
        timer->update();
        if (!timer->isCanceled()) {
            addTimerInLoop(timer);
        } else {
            // 否则删除之
            delete timer;
        }
    }
}

std::vector<Timer *> TimerQueue::popExpiredTimers() {
    std::vector<Timer *> expiredTimers;
    if (timers_.empty()) return expiredTimers;

    Date now = Date::now();
    for (int i = 0; i < timers_.size(); i++) {
        Timer *timer = timers_.top();
        // 如果定时器已经取消了, 丢弃
        if (timer->isCanceled()) {
            timers_.pop();
            delete timer; // 没用智能指针, 记得手动删除之
            continue;
        }

        // 遍历到未到期的定时器,结束
        if (now < timer->getExpiration()) {
            break;
        }

        // 已经过期的就收集起来
        expiredTimers.push_back(timer);
        timers_.pop();
    }
    return expiredTimers; // NRVO
}

Date TimerQueue::getNearestTimerExpiration() {
    assert(!timers_.empty());
    return timers_.top()->getExpiration();
}

TimerQueue::~TimerQueue() {
    timerFdChannel_.disableAllEvent();
    loop_->removeChannel(&timerFdChannel_);
}

TimerId TimerQueue::addTimer(Date when, Timer::Callback &&callback) {
    assert(Date::now() < when);  // 定时的任务应该是将来的事件
    auto newTimer = new Timer(when, std::move(callback));
    loop_->runInLoop([this, newTimer]() { this->addTimerInLoop(newTimer); });
    return newTimer->getTimerId();
}

TimerId TimerQueue::addTimer(double interval, int repeatTimes, Timer::Callback &&callback) {
    auto newTimer = new Timer(interval, repeatTimes, std::move(callback));
    loop_->runInLoop([this, newTimer]() { this->addTimerInLoop(newTimer); });
    return newTimer->getTimerId();
}

TimerId TimerQueue::addTimer(double interval, Timer::Callback &&callback, Timer::StopCondition &&stopCondition) {
    auto newTimer = new Timer(interval, std::move(callback), std::move(stopCondition));
    loop_->runInLoop([this, newTimer]() { this->addTimerInLoop(newTimer); });
    return newTimer->getTimerId();
}




