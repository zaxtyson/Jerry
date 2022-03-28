//
// Created by zaxtyson on 2022/3/14.
//

#ifndef JERRY_THREADPOOL_H
#define JERRY_THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <map>
#include <queue>
#include <thread>
#include "utils/NonCopyable.h"
#include "utils/Mutex.h"


namespace jerry::net {

enum class TaskPriority : uint8_t { LOW, NORMAL, HIGH, EMERGENCY };

class ThreadPool : NonCopyable {
  public:
    using Task = std::function<void()>;
    using TaskId = int64_t;

  public:
    /**
     * Start the thread pool
     * @param workers how many worker threads
     * @param pending_size the ReadableBytes of pending tasks, default is 0 (infinity)
     */
    void Start(size_t workers, size_t pending_size);

    /**
     * Stop all worker threads
     * the pending tasks will be discarded
     */
    void Stop();

    /**
     * Get the numbers of pending tasks
     * @return the numbers of pending tasks
     */
    size_t GetPendingTasks() const;
    TaskId SubmitTask(Task&& task);
    TaskId SubmitTask(TaskPriority priority, Task&& task);
    bool CancelTask(TaskId tid);

  private:
    void RunInThread(size_t tid);

  private:
    struct TaskStruct {
        Task task;
        TaskId tid;
        bool canceled{false};
        TaskPriority priority;
    };

    struct TaskCmp {
        bool operator()(TaskStruct* lhs, TaskStruct* rhs) { return lhs->priority < rhs->priority; }
    };

    using TaskStructQueue = std::priority_queue<TaskStruct*, std::vector<TaskStruct*>, TaskCmp>;

  private:
    size_t pending_size{};
    std::atomic<bool> stop{false};
    mutable std::mutex mtx{};
    std::condition_variable condition GUARDED_BY(mtx);
    std::map<TaskId, TaskStruct*> tasks_map GUARDED_BY(mtx);
    TaskStructQueue pending_tasks GUARDED_BY(mtx);
    std::vector<std::thread> worker_threads{};

  private:
    inline static std::atomic<TaskId> next_task_id{};
};
}  // namespace jerry::net


#endif  // JERRY_THREADPOOL_H
