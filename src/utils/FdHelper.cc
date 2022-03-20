//
// Created by zaxtyson on 2022/3/13.
//

#include "FdHelper.h"
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>  // close
#include <utils/DateTime.h>

namespace jerry::utils {
void Close(int fd) {
    if (fd < 0) {
        return;
    }
    // if fd has already closed, errno = Bad file descriptor
    int errno_bak = errno;
    close(fd);
    errno = errno_bak;
}

int CreateEventFd() {
    return eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
}


int CreateTimerFd() {
    return timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
}

void SetTimerFd(int fd, int64_t expired_us) {
    // https://man7.org/linux/man-pages/man2/timerfd_create.2.html
    // https://www.jianshu.com/p/66b3c75cae81
    struct itimerspec its {};
    expired_us = (expired_us > 100) ? expired_us : 100;
    its.it_value.tv_sec =
        static_cast<time_t>(expired_us / DateTime::kMicroSecondsPerSecond);  // seconds
    its.it_value.tv_nsec =
        static_cast<long>((expired_us % DateTime::kMicroSecondsPerSecond) * 1000);  // nanoseconds
    timerfd_settime(fd, 0, &its, nullptr);
}

}  // namespace jerry::utils