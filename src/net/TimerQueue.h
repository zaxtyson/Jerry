//
// Created by zaxtyson on 2022/3/12.
//

#ifndef JERRY_TIMERWORKER_H
#define JERRY_TIMERWORKER_H

#include <utils/DateTime.h>
#include <utils/NonCopyable.h>
#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <queue>


namespace jerry::net {

class Poller;
class Channel;

class TimerWorker : NonCopyable {
  public:
    using TimerId = int64_t;
    using Task = std::function<void()>;
    using StopCondition = std::function<bool()>;

  public:
    TimerWorker() = default;
    ~TimerWorker();

    void Init(Poller* poller);
    size_t GetTotalTimers() const;
    TimerId AddTimer(const DateTime& when, Task&& task);
    TimerId AddTimer(int64_t interval, size_t repeat_times, Task&& task);
    TimerId AddTimer(int64_t interval, StopCondition&& until, Task&& task);
    bool CancelTimer(TimerId tid);

  private:
    struct TimerStruct {
        Task task;
        StopCondition stop_condition;
        bool canceled{false};
        TimerId tid{-1};
        DateTime expire;
        size_t repeat_times;
        int64_t repeat_interval;  // us
    };

    struct TimerCmp {
        bool operator()(TimerStruct* lhs, TimerStruct* rhs) { return lhs->expire < rhs->expire; }
    };

  private:
    void OnTimeout(const DateTime& time);
    void HandleTimer(TimerStruct* timer);
    TimerId AddTimer(TimerStruct* timer);
    std::vector<TimerStruct*> PopExpiredTimers();
    int64_t GetNextTimeout() const;


  private:
    mutable std::mutex mtx{};
    Poller* poller{};
    Channel* timer_channel{};
    std::map<TimerId, TimerStruct*> timers_map{};
    std::priority_queue<TimerStruct*, std::vector<TimerStruct*>, TimerCmp> waiting_timers{};

  private:
    inline static std::atomic<TimerId> next_timer_id{};
};

using TimerId = TimerWorker::TimerId;

}  // namespace jerry::net


#endif  // JERRY_TIMERWORKER_H
