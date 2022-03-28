//
// Created by zaxtyson on 2021/10/8.
//

#include "CountDownLatch.h"

namespace jerry::utils {

CountDownLatch::CountDownLatch(int count) : count{count} {}

void CountDownLatch::Wait() {
    std::unique_lock<std::mutex> lock(mtx);
    cond.wait(lock, [this]() { return count == 0; });
}

void CountDownLatch::CountDown() {
    LockGuard lock(mtx);
    if (--count == 0) {
        cond.notify_all();
    }
}

int CountDownLatch::GetCount() const {
    LockGuard lock(mtx);
    return count;
}

}  // namespace jerry::utils