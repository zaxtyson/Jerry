//
// Created by zaxtyson on 2022/3/28.
//

#ifndef JERRY_MUTEX_H
#define JERRY_MUTEX_H

#include <mutex>

#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE(x) __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE(x)  // no-op
#endif

#define GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE(guarded_by(x))
#define PT_GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE(pt_guarded_by(x))

using LockGuard = std::lock_guard<std::mutex>;

#endif  // JERRY_MUTEX_H
