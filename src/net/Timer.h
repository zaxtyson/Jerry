//
// Created by zaxtyson on 2021/10/2.
//

#ifndef JERRY_TIMER_H
#define JERRY_TIMER_H

#include <utils/NonCopyable.h>
#include <utils/Date.h>
#include <atomic>
#include <functional>

// 定时器 ID 类型
using TimerId = uint64_t;

/**
 * 定时器类
 */
class Timer : NonCopyable {
public:
    using Callback = std::function<void()>;
    using StopCondition = std::function<bool()>;
public :

    /**
     * 构造定时器
     * @param expiration 过期时间(使用绝对时间)
     * @param callback 到期时触发的回调函数
     */
    explicit Timer(Date expiration, Callback &&callback);

    /**
     * 构造定时器
     * @param interval 每次触发的间隔时间, 单位秒
     * @param repeatTimes 运行重复触发的次数, -1表示不限制
     * @param callback 回调函数
     */
    explicit Timer(double interval, int repeatTimes, Callback &&callback);

    /**
     * 构造定时器
     * @param interval 每次触发的间隔时间, 单位秒
     * @param callback 回调函数
     * @param stopCondition 终止条件, 满足时停止定时
     */
    explicit Timer(double interval, Callback &&callback, StopCondition &&stopCondition);

    ~Timer() = default;

    /**
     * 获取定时器过期日期
     * @return
     */
    Date getExpiration() const { return expiration_; }


    /**
     * 执行定时器任务
     */
    void run();

    /**
     * 获取定时器 id
     * @return
     */
    TimerId getTimerId() const { return id_; }

    /**
     * 定时器是否已经取消
     * @return
     */
    bool isCanceled() const { return canceled_; }

    /**
     * 取消定时器
     */
    void cancel();

    /**
     * 是否重复定时
     * @return
     */
    bool repeat() const { return repeat_; }

    /**
     * 更新定时器状态, 看看是否需要继续触发
     */
    void update();

    /**
     * 用于比较两个定时器的大小, 过期时间离现在越近的越小
     * @param other
     * @return
     */
    bool operator<(const Timer &other) const { return expiration_ < other.expiration_; }

    bool operator<=(const Timer &other) const { return expiration_ <= other.expiration_; }

private:
    // Timer类共享的计数器, 用于分配id
    static std::atomic<uint64_t> nextTimerId;

public:
    // -1, 表示一直重复执行
    static int RepeatForever;

private:
    TimerId id_{nextTimerId};  // 定时器Id, 用于取消定时器
    Callback callback_;  // 到期时执行的回调函数
    Date expiration_;  // 到期的绝对时间
    bool canceled_{false};  // 被取消了吗
    StopCondition stopCondition_{nullptr}; // 该函数返回 true 则停止定时
    bool repeat_{false}; // 是否重复定时
    double interval_{0}; // 重复定时的间隔时间
    int repeatTimes_{RepeatForever}; // 剩余可重复定时次数, 减为0时自动停止定时, -1表示永不停止
};


#endif //JERRY_TIMER_H
