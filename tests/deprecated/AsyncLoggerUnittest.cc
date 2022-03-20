//
// Created by zaxtyson on 2021/10/8.
//

#include <utils/log/Logger.h>
#include "utils/DateTime.h"
#include <chrono>
#include <thread>
#include <vector>
#include <utils/log/AsyncLogger.h>
#include <memory>
#include <unistd.h>

#define LOG_NUMS 1e7

void diskSpeedTest() {
    FILE *log = fopen("./logger.text", "a+");
    auto start = std::chrono::system_clock::now();
    char buf[1024];
    const char *msg = "2021-10-12 15:12:27.000144 [FATAL] AsyncLoggerUnittest.cc:94 - this is test message [10000]\n";
    for (int i = 0; i < LOG_NUMS; i++) {
        snprintf(buf, sizeof(buf), "%s [%d]", msg, i);
        fwrite(buf, strlen(buf), 1, log);
    }

    auto end = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    printf("write to cache used %ld ms, %.2fw logger per second\n", ms, LOG_NUMS * 1.0 / ms * 1000 / 10000);

    // 等待完全写入磁盘
    fsync(fileno(log));
    end = std::chrono::system_clock::now();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    printf("writeback to disk used %ld ms, %.2fw logger per second\n", ms, LOG_NUMS * 1.0 / ms * 1000 / 10000);
}

void singleThreadTest() {
    AsyncLogger logger("./test.logger");
    Logger::setLogLevel(LogLevel::kDebug);
    Logger::setOutput(&logger);
    logger.start();

    auto start = std::chrono::system_clock::now();
    for (int i = 0; i < LOG_NUMS; i++) {
        LOG_INFO("this is test message [%d]", i);
    }

    auto end = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    printf("write to cache used %ld ms, %.2fw logger per second\n", ms, LOG_NUMS * 1.0 / ms * 1000 / 10000);

    // 等待完全写入磁盘
    logger.stop();
    end = std::chrono::system_clock::now();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    printf("writeback to disk used %ld ms, %.2fw logger per second\n", ms, LOG_NUMS * 1.0 / ms * 1000 / 10000);
}

void multiThreadTest() {
    AsyncLogger logger("./test.logger");
    Logger::setLogLevel(LogLevel::kDebug);
    Logger::setOutput(&logger);
    logger.start();

    auto start = std::chrono::system_clock::now();
    int threadNums = 4;

    std::vector<std::thread> threads;
    threads.reserve(threadNums);
    for (int i = 0; i < threadNums; i++) {
        threads.emplace_back([]() {
            for (int i = 0; i < LOG_NUMS; i++) {
                LOG_INFO("this is test message [%d]", i);
            }
        });
    }

    for (auto &th: threads) {
        th.join();
    }

    auto end = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    printf("write to cache used %ld ms, %.2fw logger per second\n", ms, LOG_NUMS * threadNums * 1.0 / ms * 1000 / 10000);

    logger.stop();
    end = std::chrono::system_clock::now();
    ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    printf("writeback to disk used %ld ms, %.2fw logger per second\n", ms,
           LOG_NUMS * threadNums * 1.0 / ms * 1000 / 10000);
}

void testLoggerLevel() {
    AsyncLogger logger("./test.logger");
    Logger::setLogLevel(LogLevel::kError);
    Logger::setOutput(&logger);
    logger.start();

    LOG_DEBUG("DEBUG message");
    LOG_INFO("INFO message");
    LOG_WARN("WARN message");
    LOG_ERROR("ERROR message");
    LOG_FATAL("FATAL message");
}

int main() {
//    diskSpeedTest();
//    testLoggerLevel();
//    singleThreadTest();
    multiThreadTest();

}