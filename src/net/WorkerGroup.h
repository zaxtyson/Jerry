//
// Created by zaxtyson on 2022/3/14.
//

#ifndef JERRY_IOWORKERGROUP_H
#define JERRY_IOWORKERGROUP_H

#include <thread>
#include "IOWorker.h"
#include <memory>
#include <vector>

namespace jerry::net {
class IOWorkerGroup {
  public:
    IOWorkerGroup() = default;
    ~IOWorkerGroup() = default;

    /**
     * Start all IOWorkers with specific config
     * @param workers how many worker threads
     * @param listen_addr the addr to be listened
     * @param callback callback for TcpConn
     */
    void Start(size_t workers, const InetAddress& listen_addr, const TcpCallback& callback);

    /**
     * Stop all workers
     * the worker will really stop after all connections are destroyed
     */
    void Stop();

    /**
     * Block the thread of caller and wait for all workers stop completely
     */
    void WaitForStop();

    /**
     * Get the number of total connections
     * @return the number of total connections
     */
    size_t GetTotalConns() const;

  private:
    std::vector<std::unique_ptr<IOWorker>> worker_set{};
    std::vector<std::thread> worker_threads{};
};
}  // namespace jerry::net


#endif  // JERRY_IOWORKERGROUP_H
