//
// Created by zaxtyson on 2022/3/12.
//

#ifndef JERRY_TIMERQUEUE_H
#define JERRY_TIMERQUEUE_H

#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include "utils/DateTime.h"
#include "utils/NonCopyable.h"


namespace jerry::net {

class Poller;
class Channel;
class IOWorker;

class TimerQueue : NonCopyable {
  public:
    using TimerId = int64_t;
    using Task = std::function<void()>;
    using StopCondition = std::function<bool()>;

  public:
    explicit TimerQueue(IOWorker* worker);
    ~TimerQueue();

    /**
     * Get the number of timers in waiting
     * @return the number of timers in waiting
     */
    size_t GetTimerNums() const;

    /**
     * Try cancel a Timer in waiting
     * @param tid the id of Timer to cancel
     * @return true if success, false if tid is invalid or Timer has already timeout
     */
    bool CancelTimer(TimerId tid);

    /**
     * Add a Timer
     * @param when when the Timer timeout
     * @param task the task to execute when Timer timeout
     * @return the id of Timer
     */
    TimerId AddTimer(const DateTime& when, Task&& task);

    /**
     * Add a repeatable Timer
     * @param interval the interval between two consecutive timeouts
     * @param repeat_times how many times do you want to repeat
     * @param task the task to execute when Timer timeout
     * @return the id of Timer
     */
    template <typename Rep, typename Period>
    inline TimerId AddTimer(std::chrono::duration<Rep, Period> interval,
                            size_t repeat_times,
                            Task&& task) {
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(interval).count();
        auto* timer = new TimerStruct();
        timer->expire = DateTime::Now().AfterMicroSeconds(us);
        timer->repeat_times = repeat_times;
        timer->repeat_interval = us;
        timer->stop_condition = nullptr;
        timer->task = task;
        return AddTimer(timer);
    }

    /**
     * Add a repeatable Timer with Stop condition
     * @param interval the interval between two consecutive timeouts
     * @param until a function to Stop repeat when it returns true
     * @param task the task to execute when Timer timeout
     * @return the id of Timer
     */
    template <typename Rep, typename Period>
    inline TimerId AddTimer(std::chrono::duration<Rep, Period> interval,
                            StopCondition&& until,
                            Task&& task) {
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(interval).count();
        auto* timer = new TimerStruct();
        timer->expire = DateTime::Now().AfterMicroSeconds(us);
        timer->repeat_times = 0;
        timer->repeat_interval = us;
        timer->task = task;
        timer->stop_condition = until;
        return AddTimer(timer);
    }

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
    IOWorker* worker{};  // the owner of this TimerQueue
    Channel* timer_channel{};
    std::map<TimerId, TimerStruct*> timers_map{};
    std::priority_queue<TimerStruct*, std::vector<TimerStruct*>, TimerCmp> waiting_timers{};

  private:
    inline static std::atomic<TimerId> next_timer_id{};
};

using TimerId = TimerQueue::TimerId;

}  // namespace jerry::net


#endif  // JERRY_TIMERQUEUE_H
