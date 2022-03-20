//
// Created by zaxtyson on 2021/9/19.
//

#include "DateTime.h"
#include <sys/time.h>  // gettimeofday

namespace jerry::utils {

DateTime DateTime::Now() {
    timeval tv{};
    gettimeofday(&tv, nullptr);
    return DateTime(tv.tv_sec * kMicroSecondsPerSecond + tv.tv_usec);
}

std::string DateTime::ToString() const {
    char buf[27] = {0};
    auto secs = static_cast<time_t>(ms_since_epoch / kMicroSecondsPerSecond);
    int ms = static_cast<int>(ms_since_epoch % kMicroSecondsPerSecond);
    tm tm_time{};
    gmtime_r(&secs, &tm_time);
    snprintf(buf,
             sizeof(buf),
             "%4d-%02d-%02d %02d:%02d:%02d.%06d",  // len = 26
             tm_time.tm_year + 1900,
             tm_time.tm_mon + 1,
             tm_time.tm_mday,
             tm_time.tm_hour + 8,
             tm_time.tm_min,
             tm_time.tm_sec,
             ms);
    return buf;
}

DateTime DateTime::AfterSeconds(double seconds) const {
    return DateTime(ms_since_epoch + static_cast<int64_t>(seconds * kMicroSecondsPerSecond));
}

}  // namespace jerry::utils
