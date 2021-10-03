//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_DATE_H
#define JERRY_DATE_H

#include <cstdint>
#include <string>
#include <sys/time.h>

/**
 * 简易的日期类
 */
class Date {
public:
    Date() = default;

    explicit Date(int64_t microSec) : microSecondsSinceEpoch_(microSec) {}

    ~Date() = default;

    /**
     * 获取当前时间
     * @return
     */
    static Date now();

    /**
     * 加上 n 秒后的时间
     * @param seconds 秒数, 可以是小数
     */
    Date addSeconds(double seconds);

    /**
     * 当前时间转换为字符串, 格式: 2021-09-19 16:20:25.944865
     * @return
     */
    std::string toString();

    /**
     * 用于比较两个时间的大小
     * @param other
     * @return
     */
    bool operator<(const Date &other) const { return microSecondsSinceEpoch_ < other.microSecondsSinceEpoch_; }

    bool operator<=(const Date &other) const { return microSecondsSinceEpoch_ <= other.microSecondsSinceEpoch_; }

    bool operator==(const Date &other) const { return microSecondsSinceEpoch_ == other.microSecondsSinceEpoch_; }

    int64_t operator-(const Date &other) const { return microSecondsSinceEpoch_ - other.microSecondsSinceEpoch_; }

public:
    static int kMicroSecondsPerSecond;
private:
    int64_t microSecondsSinceEpoch_{0};
};


#endif //JERRY_DATE_H
