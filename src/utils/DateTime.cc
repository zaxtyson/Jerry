//
// Created by zaxtyson on 2021/9/19.
//

#include "DateTime.h"
#include <sys/time.h>  // gettimeofday

namespace jerry::utils {

DateTime::DateTime(int64_t us_since_epoch) : us_since_epoch{us_since_epoch} {}

DateTime DateTime::AfterSeconds(double seconds) const {
    return DateTime(us_since_epoch + static_cast<int64_t>(seconds * kMicroSecondsPerSecond));
}

DateTime DateTime::AfterMicroSeconds(int64_t micro_seconds) const {
    return DateTime(us_since_epoch + micro_seconds);
}

bool DateTime::operator<(const DateTime& other) const {
    return us_since_epoch < other.us_since_epoch;
}

bool DateTime::operator<=(const DateTime& other) const {
    return us_since_epoch <= other.us_since_epoch;
}

bool DateTime::operator>(const DateTime& other) const {
    return us_since_epoch > other.us_since_epoch;
}

bool DateTime::operator>=(const DateTime& other) const {
    return us_since_epoch >= other.us_since_epoch;
}

bool DateTime::operator==(const DateTime& other) const {
    return us_since_epoch == other.us_since_epoch;
}

int64_t DateTime::operator-(const DateTime& other) const {
    return us_since_epoch - other.us_since_epoch;
}

DateTime DateTime::Now() {
    timeval tv{};
    gettimeofday(&tv, nullptr);
    return DateTime(tv.tv_sec * kMicroSecondsPerSecond + tv.tv_usec);
}

std::string DateTime::ToString() const {
    // TODO: Using caching to optimize the format performance
    char buf[27]{};
    auto secs = static_cast<time_t>(us_since_epoch / kMicroSecondsPerSecond);
    int us = static_cast<int>(us_since_epoch % kMicroSecondsPerSecond);
    tm tm_time{};
    gmtime_r(&secs, &tm_time);
    snprintf(buf,
             sizeof(buf),
             "%4u-%02u-%02u %02u:%02u:%02u.%06u",  // len = 26
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour + 8,
             tm_time.tm_min,
             tm_time.tm_sec,
             us);
    return buf;
}

std::string DateTime::ToGmtString() const {
    char buf[30]{};
    time_t secs = us_since_epoch / kMicroSecondsPerSecond;
    tm* gmt_time = gmtime(&secs);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %T GMT", gmt_time);
    return buf;
}

}  // namespace jerry::utils
