//
// Created by zaxtyson on 2022/3/14.
//

#include "ThreadPool.h"
#include "logger/Logger.h"

namespace jerry::net {

void ThreadPool::Start(size_t workers, size_t pending_size) {
    this->pending_size = pending_size;
    for (size_t i = 0; i < workers; i++) {
        worker_threads.emplace_back([this, i] { this->RunInThread(i); });
    }
    LOG_INFO("ThreadPool start with %zu worker(s), pending limit is %zu", workers, pending_size)
}

void ThreadPool::RunInThread(size_t tid) {
    while (!stop.load()) {
        std::unique_lock<std::mutex> lock(mtx);
        condition.wait(lock, [this] { return !pending_tasks.empty() || stop.load(); });
        if (stop.load()) {
            break;  // wakeup for stop
        }

        auto* task = pending_tasks.top();
        pending_tasks.pop();
        tasks_map.erase(task->tid);

        // if the task was canceled
        if (task->canceled) {
            delete task;
            continue;
        }

        // unlock in advance & execute the task
        lock.unlock();
        task->task();
        delete task;
    }

    // Stop loop
    while (!pending_tasks.empty()) {
        auto* task = pending_tasks.top();
        pending_tasks.pop();
        tasks_map.erase(task->tid);
        LOG_WARN("ThreadPool worker <%zu> discarded %zu pending task(s)", tid, task->tid)
        delete task;
    }
    LOG_INFO("ThreadPool worker <%zu> exited", tid)
}

void ThreadPool::Stop() {
    LOG_INFO("ThreadPool will stop soon...")
    stop.store(true);
    condition.notify_all();

    for (auto& th : worker_threads) {
        th.join();
    }
}

size_t ThreadPool::GetPendingTasks() const {
    std::lock_guard<std::mutex> lock(mtx);
    return pending_tasks.size();
}

ThreadPool::TaskId ThreadPool::SubmitTask(ThreadPool::Task&& task) {
    return SubmitTask(TaskPriority::NORMAL, std::move(task));
}

ThreadPool::TaskId ThreadPool::SubmitTask(TaskPriority priority, ThreadPool::Task&& task) {
    std::lock_guard<std::mutex> lock(mtx);

    if (pending_size > 0 && pending_tasks.size() >= pending_size) {
        return -1;
    }

    auto* ts = new TaskStruct();
    ts->task = std::move(task);
    ts->tid = next_task_id.fetch_add(1);
    ts->priority = priority;

    pending_tasks.emplace(ts);
    tasks_map.emplace(ts->tid, ts);
    condition.notify_one();  // notify worker threads

    return ts->tid;
}

bool ThreadPool::CancelTask(ThreadPool::TaskId tid) {
    if (tid >= next_task_id) {
        return false;  // tid is invalid
    }

    std::lock_guard<std::mutex> lock(mtx);
    auto target = tasks_map.find(tid);
    if (target == std::end(tasks_map)) {
        return false;  // this task has finished
    }

    target->second->canceled = true;
    return true;
}

}  // namespace jerry::net