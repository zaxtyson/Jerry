//
// Created by zaxtyson on 2022/3/13.
//

#ifndef JERRY_FDHELPER_H
#define JERRY_FDHELPER_H

#include <cstdint>

namespace jerry::utils {

/**
 * Safely close a fd even if it has been closed
 * @param fd the fd to close
 */
void Close(int fd);

int CreateEpollFd();

int CreateEventFd();

int CreateTimerFd();

/**
 * Set the timer timeout
 * @param fd timer fd
 * @param expired_us timeout in N microseconds
 */
void SetTimerFd(int fd, int64_t expired_us);

}  // namespace jerry::utils


#endif  // JERRY_FDHELPER_H
