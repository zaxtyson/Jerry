//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_NONCOPYABLE_H
#define JERRY_NONCOPYABLE_H

namespace jerry {
namespace utils {
class NonCopyable {
  public:
    NonCopyable() = default;
    ~NonCopyable() = default;

    // forbid copy
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

    // allowed move
    NonCopyable(NonCopyable&&) noexcept = default;
    NonCopyable& operator=(NonCopyable&&) noexcept = default;
};
}  // namespace utils

// for ease of use
using jerry::utils::NonCopyable;

}  // namespace jerry


#endif  // JERRY_NONCOPYABLE_H
