//
// Created by zaxtyson on 2022/3/14.
//

#ifndef JERRY_WORKERGROUP_H
#define JERRY_WORKERGROUP_H

#include <memory>
#include <thread>
#include <vector>
#include "IOWorker.h"

namespace jerry::net {
class WorkerGroup {
  public:
    WorkerGroup() = default;
    ~WorkerGroup() = default;

    /**
     * Start all IOWorkers with specific config
     * @param workers how many worker threads
     * @param listen_addr the addr_v4 to be listened
     * @param callback callback for new TcpConn
     * @param context SSLContext if enabled ssl
     */
    void Start(size_t workers,
               const InetAddress& listen_addr,
               const TcpCallback& callback,
               SslContext* context);

    /**
     * Stop all workers
     * the worker will really Stop after all connections are destroyed
     */
    void Stop();

    /**
     * Get the number of total TcpConns
     * @return the number of total TcpConns
     */
    size_t GetTotalConns() const;

    /**
     * Get the number of total Timers in waiting
     * @return the number of total Timers in waiting
     */
    size_t GetTotalTimers() const;

  private:
    std::vector<std::unique_ptr<IOWorker>> worker_set{};
    std::vector<std::thread> worker_threads{};
};
}  // namespace jerry::net


#endif  // JERRY_WORKERGROUP_H
