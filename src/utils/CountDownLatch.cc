//
// Created by zaxtyson on 2021/10/8.
//

#include "CountDownLatch.h"
#include <thread>
#include <chrono>
#include <iostream>

CountDownLatch::CountDownLatch(int count) : count_(count) {

}

void CountDownLatch::wait() {
    // https://www.zhihu.com/question/24116967
    // https://zaxtyson.cn/archives/110/#toc_12

    std::unique_lock<std::mutex> lock(mtx_);
    // 此时已经 lock(), 保护 count_, wait会自动解锁, 唤醒时拿到锁才继续往下执行
//    while (count_ > 0) {
//        cond_.wait(lock);
//    }

    // wait until condition to be true
    // wait 尝试获取锁(用于保护外部条件), 如果 lock() 成功, 执行谓词函数, 判断条件是否满足
    // 如果函数返回 true, 等待的条件满足, 程序往下执行
    // 返回 false 表示等待的条件不满足, 进程挂到 mutex 的等待队列, 同时 unlock() 等待被唤醒
    // 被唤醒时获取锁, 获取成功往下执行
    cond_.wait(lock, [this]() {
        return count_ == 0;
    });
}

void CountDownLatch::countDown() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (--count_ == 0) {
        cond_.notify_all();
    }
}

int CountDownLatch::getCount() const {
    return count_;
}
