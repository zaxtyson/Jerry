//
// Created by zaxtyson on 2021/10/8.
//

#include <utils/CountDownLatch.h>
#include <chrono>
#include <iostream>
#include <thread>

using std::cout;
using std::endl;
using namespace jerry::utils;

int main() {
    CountDownLatch latch(1);
    std::thread t1([&latch]() {
        latch.Wait();
        cout << "thread t1 runs" << endl;
    });

    std::thread t2([&latch]() {
        latch.Wait();
        cout << "thread t2 runs" << endl;
    });

    cout << "main thread preparing..." << endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    cout << "main thread prepared!" << endl;
    latch.CountDown();
    t1.join();
    t2.join();
}
