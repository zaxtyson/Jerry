//
// Created by zaxtyson on 2022/3/12.
//

#include "TimerQueue.h"
#include <cassert>
#include "Channel.h"
#include "IOWorker.h"
#include "Poller.h"
#include "logger/Logger.h"
#include "utils/FdHelper.h"

namespace jerry::net {

TimerQueue::TimerQueue(IOWorker* worker) {
    this->worker = worker;
    int timer_fd = utils::CreateTimerFd();
    if (timer_fd == -1) {
        LOG_FATAL("Create timer fd failed")
    }
    LOG_DEBUG("Create timer fd = %d success", timer_fd)

    timer_channel = new Channel(timer_fd);
    timer_channel->SetPoller(worker->GetPoller());

    ChannelCallback timer_callback;
    timer_callback.OnReadable = [this](const DateTime& time) { this->OnTimeout(time); };
    timer_channel->SetCallback(std::move(timer_callback));
    timer_channel->AddToPoller();
    timer_channel->ActivateReading();
}

TimerQueue::~TimerQueue() {
    delete timer_channel;
}

int64_t TimerQueue::GetNextTimeout() const {
    LockGuard lock(mtx);
    if (waiting_timers.empty()) {
        return -1;  // no timers
    }
    auto timeout = waiting_timers.top()->expire - DateTime::Now();
    return timeout > 0 ? timeout : 0;
}

std::vector<TimerQueue::TimerStruct*> TimerQueue::PopExpiredTimers() {
    std::vector<TimerStruct*> expired_timers;
    if (waiting_timers.empty()) {
        return expired_timers;
    }

    LockGuard lock(mtx);
    auto now = DateTime::Now();
    for (size_t i = 0; i < waiting_timers.size(); i++) {
        auto* timer = waiting_timers.top();

        // traversed an unexpired timer, stop_loop
        if (timer->expire > now) {
            break;
        }

        waiting_timers.pop();
        timers_map.erase(timer->tid);
        expired_timers.emplace_back(timer);
    }

    return expired_timers;
}

void TimerQueue::OnTimeout([[maybe_unused]] const DateTime& time) {
    auto timer_fd = timer_channel->GetFd();

    // drop the data to prevent busy-loop
    uint64_t value;
    [[maybe_unused]] size_t rd = read(timer_fd, &value, sizeof(value));
    assert(rd == sizeof(value));

    auto expired_timers = PopExpiredTimers();
    LOG_DEBUG("Pop %zu expired timer(s), timer fd = %d", expired_timers.size(), timer_fd)

    // update time of next timeout
    auto next_timeout = GetNextTimeout();
    if (next_timeout != -1) {
        LOG_DEBUG("The timer fd = %d will wakeup after %ldus", timer_fd, next_timeout)
        utils::SetTimerFd(timer_channel->GetFd(), next_timeout);
    }

    // handle expired timers
    for (auto* timer : expired_timers) {
        HandleTimer(timer);
    }
}

void TimerQueue::HandleTimer(TimerQueue::TimerStruct* timer) {
    // task canceled
    if (timer->canceled) {
        LOG_DEBUG("Timer tid = %ld was canceled, remove it", timer->tid)
        std::lock_guard<std::mutex> lock(mtx);
        timers_map.erase(timer->tid);
        delete timer;
        return;
    }

    // task stop_loop
    if (timer->stop_condition && timer->stop_condition()) {
        LOG_DEBUG("Timer tid = %ld meets the stopping condition, remove it", timer->tid)
        LockGuard lock(mtx);
        timers_map.erase(timer->tid);
        delete timer;
        return;
    }

    // execute task
    if (timer->task) {
        LOG_DEBUG("Timer tid = %ld timeout, execute the bound task...", timer->tid)
        timer->task();
    }

    // if task repeatable
    if (timer->repeat_times > 0) {
        // update and reuse this timer
        timer->repeat_times--;
        timer->expire = DateTime::Now().AfterMicroSeconds(timer->repeat_interval);
        AddTimer(timer);
        return;
    }

    // single task or repeat_timers == 0
    delete timer;
}

TimerQueue::TimerId TimerQueue::AddTimer(TimerQueue::TimerStruct* timer) {
    if (timer->expire <= DateTime::Now()) {
        return -1;
    }

    LockGuard lock(mtx);
    // update the wakeup time of timer_fd to the earliest triggered timer
    if (waiting_timers.empty() || timer->expire < waiting_timers.top()->expire) {
        int64_t next_timeout = timer->expire - DateTime::Now();
        utils::SetTimerFd(timer_channel->GetFd(), next_timeout);
        LOG_DEBUG(
            "The timer fd = %d will wakeup after %ldus", timer_channel->GetFd(), next_timeout)
    }

    // id is invalid
    if (timer->tid == -1) {
        timer->tid = next_timer_id.fetch_add(1);
    }

    waiting_timers.emplace(timer);
    timers_map.emplace(timer->tid, timer);
    return timer->tid;
}

TimerQueue::TimerId TimerQueue::AddTimer(const DateTime& when, TimerQueue::Task&& task) {
    auto* timer = new TimerStruct();
    timer->expire = when;
    timer->repeat_times = 0;
    timer->stop_condition = nullptr;
    timer->task = std::move(task);

    return AddTimer(timer);
}

bool TimerQueue::CancelTimer(TimerQueue::TimerId tid) {
    LOG_DEBUG("Timer tid = %ld is marked as canceled", tid)
    LockGuard lock(mtx);
    auto target = timers_map.find(tid);
    if (target == std::end(timers_map)) {
        return false;
    }
    target->second->canceled = true;
    return true;
}

size_t TimerQueue::GetTimerNums() const {
    LockGuard lock(mtx);
    return waiting_timers.size();
}

}  // namespace jerry::net