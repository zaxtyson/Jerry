//
// Created by zaxtyson on 2021/10/2.
//

#ifndef JERRY_TIMERQUEUE_H
#define JERRY_TIMERQUEUE_H

#include <queue>
#include <map>
#include <net/Channel.h>
#include <net/Timer.h>
#include <utils/NonCopyable.h>

class EventLoop;

/**
 * 用于组织定时器, 每个事件循环都有一个
 */
class TimerQueue : NonCopyable {
public:
    // 用于容器排序
    struct cmp {
        bool operator()(Timer *lhs, Timer *rhs) { return *rhs < *lhs; }
    };

    using TimerContainer = std::priority_queue<Timer *, std::vector<Timer *>, cmp>;  // 小根堆
    using TimerIdMap = std::map<TimerId, Timer *>;

public:
    explicit TimerQueue(EventLoop *loop);

    ~TimerQueue();

    TimerId addTimer(Date when, Timer::Callback &&callback);

    TimerId addTimer(double interval, int repeatTimes, Timer::Callback &&callback);

    TimerId addTimer(double interval, Timer::Callback &&callback, Timer::StopCondition &&stopCondition);

    /**
     * 取消一个定时器
     * @param timerId 定时器 ID
     */
    void cancelTimer(TimerId timerId);

private:
    /**
     * 当 timerfd 可读时, 说明有定时器超时, 调用本函数处理
     */
    void handleTimeoutEvent();

    /**
     * 从容器中弹出已经过期的定时器
     * @param date 该时间以前的定时器将被弹出
     * @return
     */
    std::vector<Timer *> popExpiredTimers();

    /**
     * 获取超时时间最近的计时器的过期时间
     * @return
     */
    Date getNearestTimerExpiration();

    // 确保在事件循环中操作, 保证线程安全
    void addTimerInLoop(Timer *newTimer);

    void cancelInLoop(TimerId timerId);

private:
    EventLoop *loop_;
    int timerFd_;
    Channel timerFdChannel_;
    TimerContainer timers_{};
    TimerIdMap timerIdMap_{};
};


#endif //JERRY_TIMERQUEUE_H
