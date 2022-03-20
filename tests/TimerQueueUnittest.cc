//
// Created by zaxtyson on 2021/10/2.
//

#include <net/TimerQueue.h>
#include <net/EventLoop.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;
using namespace std::chrono_literals;

template<typename T>
std::function<void()> show(T i) {
    return [i]() {
        cout << "Timeout: " << i << endl;
    };
}


void testRawTimeQueue() {
    // EventLoop 还没有集成 TimerQueue 时, 测试功能使用
    EventLoop loop;
    TimerQueue timerQueue(&loop);

    Date now = Date::now();
    timerQueue.addTimer(now.AfterSeconds(1), show("1s"));
    timerQueue.addTimer(1, 3, show("run per 1s for 3 times"));
    timerQueue.addTimer(1.5, Timer::RepeatForever, show("run forever"));
    int i = 0;
    timerQueue.addTimer(3, show("run with condition"), [&i]() { return ++i > 3; });


    cout << "Hello, main" << endl;
    loop.loop();
}

void testRunAt() {
    EventLoop loop;

    loop.runAt(Date::now().AfterSeconds(3), show("3s"));
    loop.runAt(Date::now().AfterSeconds(0.5), show("0.5s"));
    loop.runAt(Date::now().AfterSeconds(5), show("5s"));
    loop.runAt(Date::now().AfterSeconds(1), show("1s"));

    std::thread t1([&loop]() {
        std::this_thread::sleep_for(1s);
        Date d = Date::now();
        loop.runAt(d.AfterSeconds(1), show("other thread 1"));
        loop.runAt(d.AfterSeconds(3), show("other thread 2"));
    });
    t1.detach();
    loop.loop();
}

void testRunEveryWithStopCondition() {
    EventLoop loop;

    int i = 1;

    auto task = [&i]() {
        cout << i << endl;
        i++;
    };

    auto stop = [&i]() { return i == 4; };

    loop.runEvery(1, task, stop);

    loop.loop();
}

void testRunEveryWithRepeatTimes() {
    EventLoop loop;

    loop.runEvery(3, Timer::RepeatForever, show("run per 3s forever"));
    loop.runEvery(1, 3, show("run per 1s for 3 times"));

    loop.loop();
}


int main() {
//    testRawTimeQueue();
//    testRunAt();
//    testRunEveryWithRepeatTimes();
    testRunEveryWithStopCondition();
}