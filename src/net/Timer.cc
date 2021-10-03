//
// Created by zaxtyson on 2021/10/2.
//

#include "Timer.h"

std::atomic<uint64_t> Timer::nextTimerId = 0;  // 全局静态计数器
int Timer::RepeatForever = -1;


Timer::Timer(Date expiration, Callback &&callback) :
        expiration_(expiration),
        callback_(callback) {
    ++nextTimerId;  // 创建一个定时器, 计数+1
}


Timer::Timer(double interval, int repeatTimes, Timer::Callback &&callback) :
        expiration_(Date::now().addSeconds(interval)),  // 第一次触发设在第一个interval之后
        interval_(interval),
        repeat_(true),
        repeatTimes_(repeatTimes),
        callback_(callback) {
    ++nextTimerId;
}

void Timer::update() {
    if (!repeat()) {
        cancel();  // 单次定时任务
        return;
    }

    if (stopCondition_ && stopCondition_()) {
        cancel();  // 满足停止条件, 取消重复定时
        return;
    }

    if (repeatTimes_ > 0) {
        --repeatTimes_;
    }

    if (repeatTimes_ == 0) {
        cancel(); // 不用在执行了
        return;
    }

    // 设置到期时间为下一个间隔
    expiration_ = expiration_.addSeconds(interval_);
}

Timer::Timer(double interval, Timer::Callback &&callback, Timer::StopCondition &&stopCondition) :
        expiration_(Date::now().addSeconds(interval)),
        interval_(interval),
        repeat_(true),
        callback_(callback),
        stopCondition_(stopCondition) {
    ++nextTimerId;
}

void Timer::run() {
    // 定时器有效才执行
    if (!isCanceled() && callback_) {
        callback_();
    }
}

void Timer::cancel() {
    repeat_ = false;
    canceled_ = true;
}
