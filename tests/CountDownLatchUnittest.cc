//
// Created by zaxtyson on 2021/10/8.
//

#include "utils/CountDownLatch.h"
#include <thread>
#include <chrono>
#include <iostream>

using std::cout;
using std::endl;

int main() {
    CountDownLatch latch(1);
    std::thread t1([&latch]() {
        latch.wait();
        cout << "thread t1 run" << endl;
    });

    std::thread t2([&latch]() {
        latch.wait();
        cout << "thread t2 run" << endl;
    });

    cout << "main thread preparing..." << endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    cout << "main thread prepared!" << endl;
    latch.countDown();
    t1.join();
    t2.join();
}
