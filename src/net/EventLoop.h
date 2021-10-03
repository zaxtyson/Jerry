//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_EVENTLOOP_H
#define JERRY_EVENTLOOP_H

#include <utils/NonCopyable.h>
#include <net/Channel.h>
#include <net/Poller.h>
#include <net/Timer.h>
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>

class TimerQueue;

/**
 * 事件循环类, 每一个事件循环都运行在一个单独的线程内,
 * 每个事件循环都有一个 poller, 用于管理 fd
 * 同时带有一个 taskList, 用于跨线程添加任务,
 * 使用 eventfd 实现跨线程唤醒
 * 同时带有一个 TimerQueue, 用于设置定时任务
 */
class EventLoop : NonCopyable {
public:
    using Task = std::function<void()>;
    using TaskList = std::queue<Task>;
    using ChannelList = std::vector<Channel *>;
public:
    EventLoop();

    ~EventLoop();

    /**
     * 启动事件循环
     */
    void loop();

    /**
     * 获取 Poller 的文件描述符
     * @return
     */
    int getPollerFd() const { return poller_->getFd(); }

    /**
     * 处理完剩下的事情后, 优雅地关闭事件循环
     */
    void quit() { quit_ = true; }

    /**
     * 断言调用者在当前事件循环所在线程
     * 如果断言失败, 终止程序
     */
    void assertInLoopThread();


    /**
     * 添加一个 Channel 到当前事件循环的 Poller
     * @param channel
     */
    void addChannel(Channel *channel) { poller_->addChannel(channel); }

    /**
     * 删除当前事件循环中的某个 Channel
     * @param channel
     */
    void removeChannel(Channel *channel) { poller_->removeChannel(channel); }

    /**
     * 更新当前事件循环中的某个 Channel
     * @param channel
     */
    void updateChannel(Channel *channel) { poller_->updateChannel(channel); }

    /**
     * 跨线程唤醒当前事件循环
     */
    void wakeup() const;

    /**
     * 在当前事件循环中执行任务
     * 使用 eventfd 机制跨线程唤醒本事件循环
     * @param task
     */
    void runInLoop(Task &&task);

    /**
     * 在指定时间执行一次任务
     * @param when 未来的每个时间点
     * @param callback 回调函数, 请确保回调绑定的指针在函数触发时仍然有效
     * @return 定时器 Id
     */
    TimerId runAt(Date when, Timer::Callback &&callback);

    /**
     * 从现在开始每隔一段时间触发一次(第一次触发在一个间隔时间后)
     * @param interval 时间间隔, 秒(可为小数)
     * @param repeatTimes 重复次数, Timer::RepeatForever 表示永不停止
     * @param callback 回调函数, 请确保回调绑定的指针在函数触发时仍然有效
     * @return 定时器 Id
     */
    TimerId runEvery(double interval, int repeatTimes, Timer::Callback &&callback);

    /**
     * 从现在开始每隔一段时间触发一次(第一次触发在一个间隔时间后)
     * @param interval 时间间隔, 秒(可为小数)
     * @param callback  回调函数, 请确保回调绑定的指针在函数触发时仍然有效
     * @param stopCondition 终止条件, 满足时停止定时
     * @return
     */
    TimerId runEvery(double interval, Timer::Callback &&callback, Timer::StopCondition &&stopCondition);

    /**
     * 取消执行任务
     * @param timerId 定时器任务的id
     */
    void cancel(TimerId timerId);

private:
    // eventfd 可读时被调用
    void handleWakeupEvent() const;

    void executeTask();

private:
    TaskList taskList_{};  // 任务列表, 可跨线程添加任务
    ChannelList activeChannelList_{};  // poller 返回的活跃事件列表
    std::unique_ptr<Poller> poller_;  // 我们使用 Epoll 作为 Poller
    bool quit_{false};  // 是否退出, 用于优雅关闭
    std::thread::id threadId_{std::this_thread::get_id()};  // 当前 EventLoop 所在的线程 id
    std::mutex mtx_{};  // 用于保护任务列表

    int wakeupFd_{-1};  // 使用 eventfd 机制跨线程唤醒当前事件循环
    Channel wakeupChannel_{};

    std::unique_ptr<TimerQueue> timerQueue_;
};


#endif //JERRY_EVENTLOOP_H
