//
// Created by zaxtyson on 2022/3/12.
//

#include "TimerWorker.h"
#include <utils/FdHelper.h>
#include <cassert>
#include "Channel.h"
#include "Poller.h"

namespace jerry::net {

void TimerWorker::Init(Poller* poller) {
    this->poller = poller;
    int timer_fd = utils::CreateTimerFd();
    assert(timer_fd > 0);

    timer_channel = new Channel(timer_fd);
    timer_channel->SetPoller(poller);

    ChannelCallback callback;
    callback.OnReadable = [this](const DateTime& time) { this->OnTimeout(time); };
    timer_channel->SetCallback(callback);
    timer_channel->ActivateReading();
}

TimerWorker::~TimerWorker() {
    delete timer_channel;
}

int64_t TimerWorker::GetNextTimeout() const {
    std::lock_guard<std::mutex> lock(mtx);
    auto timeout = waiting_timers.top()->expire - DateTime::Now();
    return timeout > 0 ? timeout : 0;
}

std::vector<TimerWorker::TimerStruct*> TimerWorker::PopExpiredTimers() {
    std::vector<TimerStruct*> expired_timers;
    if (waiting_timers.empty()) {
        return expired_timers;
    }

    std::lock_guard<std::mutex> lock(mtx);
    auto now = DateTime::Now();
    for (auto i = 0; i < waiting_timers.size(); i++) {
        auto* timer = waiting_timers.top();

        // traversed an unexpired timer, stop
        if (timer->expire > now) {
            break;
        }

        waiting_timers.pop();
        timers_map.erase(timer->tid);
        expired_timers.emplace_back(timer);
    }

    return expired_timers;
}

void TimerWorker::OnTimeout(const DateTime& time) {
    // drop the data to prevent busy-loop
    uint64_t n;
    read(timer_channel->GetFd(), &n, sizeof(n));

    auto expired_timers = PopExpiredTimers();

    // update time of next timeout
    utils::SetTimerFd(timer_channel->GetFd(), GetNextTimeout());

    // handle expired timers
    for (auto* timer : expired_timers) {
        HandleTimer(timer);
    }
}

void TimerWorker::HandleTimer(TimerWorker::TimerStruct* timer) {
    // task canceled
    if (timer->canceled) {
        std::lock_guard<std::mutex> lock(mtx);
        timers_map.erase(timer->tid);
        delete timer;
        return;
    }

    // task stop
    if (timer->stop_condition && timer->stop_condition()) {
        std::lock_guard<std::mutex> lock(mtx);
        timers_map.erase(timer->tid);
        delete timer;
        return;
    }

    // execute task
    timer->task();

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

TimerWorker::TimerId TimerWorker::AddTimer(TimerWorker::TimerStruct* timer) {
    if (timer->expire <= DateTime::Now()) {
        return -1;
    }

    std::lock_guard<std::mutex> lock(mtx);
    // update the wakeup time of timer_fd to the earliest triggered timer
    if (waiting_timers.empty() || timer->expire < waiting_timers.top()->expire) {
        int64_t next_timeout = timer->expire - DateTime::Now();
        utils::SetTimerFd(timer_channel->GetFd(), next_timeout);
    }

    // id is invalid
    if (timer->tid == -1) {
        timer->tid = next_timer_id.fetch_add(1);
    }

    waiting_timers.emplace(timer);
    timers_map.emplace(timer->tid, timer);
    return timer->tid;
}

TimerWorker::TimerId TimerWorker::AddTimer(const DateTime& when, TimerWorker::Task&& task) {
    auto* timer = new TimerStruct();
    timer->expire = when;
    timer->repeat_times = 0;
    timer->stop_condition = nullptr;
    timer->task = std::move(task);

    return AddTimer(timer);
}

TimerWorker::TimerId TimerWorker::AddTimer(int64_t interval,
                                           size_t repeat_times,
                                           TimerWorker::Task&& task) {
    auto* timer = new TimerStruct();
    timer->expire = DateTime::Now().AfterMicroSeconds(interval);
    timer->repeat_times = repeat_times;
    timer->repeat_interval = interval;
    timer->stop_condition = nullptr;
    timer->task = std::move(task);

    return AddTimer(timer);
}

TimerWorker::TimerId TimerWorker::AddTimer(int64_t interval,
                                           TimerWorker::StopCondition&& until,
                                           TimerWorker::Task&& task) {
    auto* timer = new TimerStruct();
    timer->expire = DateTime::Now().AfterMicroSeconds(interval);
    timer->repeat_times = 0;
    timer->repeat_interval = interval;
    timer->stop_condition = nullptr;
    timer->task = std::move(task);
    timer->stop_condition = std::move(until);

    return AddTimer(timer);
}

bool TimerWorker::CancelTimer(TimerWorker::TimerId tid) {
    std::lock_guard<std::mutex> lock(mtx);
    auto target = timers_map.find(tid);
    if (target == std::end(timers_map)) {
        return false;
    }
    target->second->canceled = true;
    return true;
}

size_t TimerWorker::GetTotalTimers() const {
    std::lock_guard<std::mutex> lock(mtx);
    return waiting_timers.size();
}


}  // namespace jerry::net