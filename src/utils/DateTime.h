//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_DATETIME_H
#define JERRY_DATETIME_H

#include <cstdint>
#include <string>

namespace jerry {
namespace utils {

class DateTime {
  public:
    DateTime() = default;
    explicit DateTime(int64_t micro_sec_since_epoch) : ms_since_epoch{micro_sec_since_epoch} {}
    ~DateTime() = default;

    /**
     * Get current datetime
     * @return datetime Now
     */
    static DateTime Now();

    /**
     * Get datetime after n seconds
     * @param seconds n seconds
     * @return new datetime
     */
    DateTime AfterSeconds(double seconds) const;

    /**
     * Convert current time to string, format:  2021-09-19 16:20:25.944865
     * @return datetime string
     */
    std::string ToString() const;

  public:
    bool operator<(const DateTime& other) const { return ms_since_epoch < other.ms_since_epoch; }
    bool operator<=(const DateTime& other) const { return ms_since_epoch <= other.ms_since_epoch; }
    bool operator==(const DateTime& other) const { return ms_since_epoch == other.ms_since_epoch; }
    int64_t operator-(const DateTime& other) const { return ms_since_epoch - other.ms_since_epoch; }

  public:
    inline static int kMicroSecondsPerSecond{1000000};

  private:
    int64_t ms_since_epoch{};
};
}  // namespace utils

using utils::DateTime;

}  // namespace jerry

#endif  // JERRY_DATETIME_H
