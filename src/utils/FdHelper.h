//
// Created by zaxtyson on 2021/10/2.
//

#ifndef JERRY_FDHELPER_H
#define JERRY_FDHELPER_H

#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <utils/Date.h>
#include <utils/Logger.h>
#include <cassert>

/**
 * 创建一个 eventfd, 用于跨线程通信
 * @return
 */
static int createEventFd() {
    int evtFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtFd < 0) {
        LOG_FATAL("Cannot create event fd!");
        exit(1);
    }
    return evtFd;
}

/**
 * 创建一个 timerfd, 用于定时器定时
 * @return
 */
static int createTimerFd() {
    int timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerFd < 0) {
        LOG_FATAL("Cannot create event fd!");
        exit(1);
    }
    return timerFd;
}

/**
 * 设置 timerfd 的触发时间
 * @param timerFd
 * @param microSeconds 距离当前时间的微秒数
 */
static void setTimerFd(int timerFd, int64_t microSeconds) {
    // https://man7.org/linux/man-pages/man2/timerfd_create.2.html
    // https://www.jianshu.com/p/66b3c75cae81
    assert(microSeconds >= 0);

    struct itimerspec its{};

    microSeconds = (microSeconds > 100) ? microSeconds : 100;  // 控制下精度, 没必要太细
    its.it_value.tv_sec = static_cast<time_t>(microSeconds / Date::kMicroSecondsPerSecond); /* Seconds */
    its.it_value.tv_nsec = static_cast<long>((microSeconds % Date::kMicroSecondsPerSecond) * 1000); /* Nanoseconds */
    timerfd_settime(timerFd, 0, &its, nullptr);
}

#endif //JERRY_FDHELPER_H
