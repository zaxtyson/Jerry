//
// Created by zaxtyson on 2021/10/8.
//

#ifndef JERRY_COUNTDOWNLATCH_H
#define JERRY_COUNTDOWNLATCH_H

#include <condition_variable>
#include <mutex>
#include "utils/NonCopyable.h"

namespace jerry::utils {

class CountDownLatch : NonCopyable {
  public:
    explicit CountDownLatch(int count);
    ~CountDownLatch() = default;

    void Wait();
    void CountDown();
    int GetCount() const;

  private:
    std::mutex mtx{};
    std::condition_variable cond{};
    int count;
};

}  // namespace jerry::utils

#endif  // JERRY_COUNTDOWNLATCH_H
