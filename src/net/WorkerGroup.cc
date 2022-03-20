//
// Created by zaxtyson on 2022/3/14.
//

#include "WorkerGroup.h"
#include "IOWorker.h"
#include "logger/Logger.h"

namespace jerry::net {

void WorkerGroup::Start(size_t workers,
                        const InetAddress& listen_addr,
                        const TcpCallback& callback) {
    for (size_t i = 0; i < workers; i++) {
        auto* worker = new IOWorker(i, listen_addr, callback);
        worker_set.emplace_back(worker);
        worker_threads.emplace_back([worker] { worker->Loop(); });
    }
    LOG_INFO("WorkerGroup start with %zu worker(s)", workers)
}

void WorkerGroup::Stop() {
    LOG_INFO("WorkerGroup will stop soon...")
    for (auto& worker : worker_set) {
        worker->Stop();
    }

    for (auto& th : worker_threads) {
        th.join();
    }
}

size_t WorkerGroup::GetTotalConns() const {
    size_t total = 0;
    for (auto& worker : worker_set) {
        total += worker->GetConnNums();
    }
    return total;
}

size_t WorkerGroup::GetTotalTimers() const {
    size_t total = 0;
    for (auto& worker : worker_set) {
        total += worker->GetTimerNums();
    }
    return total;
}

}  // namespace jerry::net