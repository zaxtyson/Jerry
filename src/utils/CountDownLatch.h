//
// Created by zaxtyson on 2021/10/8.
//

#ifndef JERRY_COUNTDOWNLATCH_H
#define JERRY_COUNTDOWNLATCH_H

#include <mutex>
#include <condition_variable>
#include <utils/NonCopyable.h>

class CountDownLatch : NonCopyable {
public :
    explicit CountDownLatch(int count);

    ~CountDownLatch() = default;

    void wait();

    void countDown();

    int getCount() const;
private:
    std::mutex mtx_{};
    std::condition_variable cond_;
    int count_;
};


#endif //JERRY_COUNTDOWNLATCH_H
