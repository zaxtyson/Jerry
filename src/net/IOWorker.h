//
// Created by zaxtyson on 2022/3/12.
//

#ifndef JERRY_IOWORKER_H
#define JERRY_IOWORKER_H

#include <mutex>
#include <set>
#include "TcpConn.h"
#include "utils/NonCopyable.h"

namespace jerry::net {

class Poller;
class Acceptor;
class TimerQueue;

class IOWorker : NonCopyable {
  public:
    explicit IOWorker(size_t tid, const InetAddress& listen_addr, const TcpCallback& callback);
    ~IOWorker();

    /**
     * Start the IOWorker
     * it will create an acceptor and listen on `listen_ip`
     * thread will be block at the event loop until `Stop` be called
     * the worker will be closed after all connections disconnected
     */
    void Loop();

    /**
     * Stop the IOWorker
     */
    void Stop();

    /**
     * Get the poller in this IOWorker
     * @return the Poller
     */
    Poller* GetPoller();

    /**
     * Get the TimerQueue in this IOWorker
     * @return the TimerQueue
     */
    TimerQueue* GetTimerQueue();

    /**
     * Get the number of TcpConns
     * @return the number of TcpConns
     */
    size_t GetConnNums() const;

    /**
     * Get the number of Timers in waiting
     * @return the number of Timers in waiting
     */
    size_t GetTimerNums() const;

    /**
     * Add a TcpConn into tcp_conns
     * it will make a shared_ptr for the TcpConn to ensure that the
     * lifecycle of the TcpConn object is long enough
     * @param conn the new TcpConn created by Acceptor
     */
    void AddTcpConn(TcpConn* conn);

    /**
     * Remove the shared_ptr of TcpConn in tcp_conns
     * the TcpConn will be destroyed when the last shared_ptr destroyed
     * may be someone else holding the shared_ptr of this TcpConn,
     * so it may NOT be destroyed immediately
     * @param conn the TcpConn to remove
     */
    void RemoveTcpConn(const TcpConnPtr& conn);

  private:
    size_t tid{};  // thread id in IOWorkerGroup
    Poller* poller{};
    TimerQueue* timer_queue{};
    Acceptor* acceptor{};
    TcpConnSet tcp_conns{};  // store TcpConn
    std::atomic<bool> stop_loop{false};
};
}  // namespace jerry::net


#endif  // JERRY_IOWORKER_H
