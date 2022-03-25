//
// Created by zaxtyson on 2022/3/12.
//

#include "IOWorker.h"
#include <cassert>
#include <csignal>  // signal
#include <thread>
#include "Acceptor.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"
#include "logger/Logger.h"

namespace jerry::net {

IOWorker::IOWorker(size_t tid, const InetAddress& listen_addr, const TcpCallback& callback) {
    this->tid = tid;
    this->poller = new Poller();
    this->acceptor = new Acceptor(this, listen_addr, callback);
    this->timer_queue = new TimerQueue(this);

    assert(this->poller);
    assert(this->acceptor);
    assert(this->timer_queue);
}

IOWorker::~IOWorker() {
    delete poller;
    delete acceptor;
    delete timer_queue;
}

void IOWorker::Loop() {
    // SIGPIPE will be raised when we read/write a fd while it was closed
    // https://stackoverflow.com/questions/21687695/getting-sigpipe-with-non-blocking-sockets-is-this-normal
    struct sigaction action {};
    action.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &action, nullptr);

    // Start the event loop
    Poller::ChannelList active_channels;

    while (!stop_loop.load()) {
        active_channels.clear();
        auto time = poller->Poll(active_channels);  // thread block here
        for (auto* channel : active_channels) {
            channel->HandleEvent(time);
        }
    }

    // Stop the worker
    // shutdown the established TcpConns
    auto conns = tcp_conns.Size();
    if (conns > 0) {
        LOG_WARN("IOWorker <%zu> will exist after close %zu established connection(s)", tid, conns)
    }
    tcp_conns.LockForEach([](const TcpConnPtr& conn) { conn->ForceClose(); });

    // TcpConn object will be removed from tcp_conns when it disconnected
    // waiting for all connections disconnected here
    while (!tcp_conns.Empty()) {
        LOG_DEBUG("IOWorker <%zu> is waiting for TcpConn(s) disconnection...", tid)
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LOG_INFO("IOWorker <%zu> existed", tid)
}

void IOWorker::Stop() {
    stop_loop.store(true);
    poller->Wakeup();
}

void IOWorker::AddTcpConn(TcpConn* conn) {
    tcp_conns.Add(conn);
}

void IOWorker::RemoveTcpConn(const TcpConnPtr& conn) {
    tcp_conns.Remove(conn);
}

size_t IOWorker::GetConnNums() const {
    return tcp_conns.Size();
}

size_t IOWorker::GetTimerNums() const {
    return timer_queue->GetTimerNums();
}

Poller* IOWorker::GetPoller() {
    return poller;
}

TimerQueue* IOWorker::GetTimerQueue() {
    return timer_queue;
}

SslContext* IOWorker::GetSslContext() const {
    return ssl_ctx;
}

void IOWorker::SetSslContext(SslContext* context) {
    this->ssl_ctx = context;
}

}  // namespace jerry::net