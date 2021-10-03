//
// Created by zaxtyson on 2021/9/19.
//

#include "Date.h"

int Date::kMicroSecondsPerSecond = 1e6;

Date Date::now() {
    timeval tv{};
    gettimeofday(&tv, nullptr);
    return Date(tv.tv_sec * kMicroSecondsPerSecond + tv.tv_usec);
}

std::string Date::toString() {
    char buf[128] = {0};
    time_t secs = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    int ms = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
    tm tm_time{};
    gmtime_r(&secs, &tm_time);
    snprintf(buf,
             sizeof(buf),
             "%4d-%02d-%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour + 8,
             tm_time.tm_min,
             tm_time.tm_sec,
             ms);
    return buf;
}

Date Date::addSeconds(double seconds) {
    return Date(microSecondsSinceEpoch_ + static_cast<int64_t>(seconds * kMicroSecondsPerSecond));
}






