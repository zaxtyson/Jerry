//
// Created by zaxtyson on 2022/3/14.
//

#include "IOWorkerGroup.h"

namespace jerry::net {

void IOWorkerGroup::Start(size_t workers,
                          const InetAddress& listen_addr,
                          const TcpCallback& callback) {
    for (auto i = 0; i < workers; i++) {
        auto* worker = new IOWorker(listen_addr, callback);
        worker_set.emplace_back(worker);
        worker_threads.emplace_back([worker] { worker->Loop(); });
    }
}

void IOWorkerGroup::Stop() {
    for (auto& worker : worker_set) {
        worker->Stop();
    }
}

void IOWorkerGroup::WaitForStop() {
    for (auto& th : worker_threads) {
        th.join();
    }
}

size_t IOWorkerGroup::GetTotalConns() const {
    size_t total = 0;
    for (auto& worker : worker_set) {
        total += worker->GetTotalConns();
    }
    return total;
}

}  // namespace jerry::net