//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_NONCOPYABLE_H
#define JERRY_NONCOPYABLE_H

/**
 * 工具类, 禁止子类被拷贝
 */
class NonCopyable {
protected:
    NonCopyable() = default;

    ~NonCopyable() = default;

    // 删除拷贝构造和拷贝赋值
    NonCopyable(const NonCopyable &) = delete;

    NonCopyable &operator=(const NonCopyable &) = delete;

    // 允许移动语义
    NonCopyable(NonCopyable &&) noexcept = default;

    NonCopyable &operator=(NonCopyable &&) noexcept = default;
};


#endif //JERRY_NONCOPYABLE_H
