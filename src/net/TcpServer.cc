//
// Created by zaxtyson on 2022/3/12.
//

#include "TcpServer.h"
#include <csignal>
#include "Channel.h"
#include "logger/Logger.h"
#include "net/TimerQueue.h"  // for user convenience

namespace jerry::net {

void TcpServer::Config(const ServerConfig& config) {
    this->config = config;

    if (config.ssl.use_ssl) {
        ssl_ctx = new SslContext();
        if (config.ssl.disable_old_ssl_version) {
            ssl_ctx->DisableOldVersion();
        }
        if (config.ssl.enable_validation) {
            ssl_ctx->EnableValidation();
        }
        ssl_ctx->SetCertPath(config.ssl.cert_file, config.ssl.private_key_file);
    }
}

TcpServer::~TcpServer() {
    delete ssl_ctx;
}

size_t TcpServer::GetTotalConns() const {
    return worker_group.GetTotalConns();
}

size_t TcpServer::GetTotalTimers() const {
    return worker_group.GetTotalTimers();
}

size_t TcpServer::GetPendingTasks() const {
    return thread_pool.GetPendingTasks();
}

void TcpServer::Serve() {
    // this callback will be bind to all new TcpConn
    TcpCallback callback;
    callback.OnConnected = [this](TcpConn* conn, const DateTime& time) {
        if (limiter && limiter->ReachConnectionLimits(conn)) {
            conn->Shutdown();
            return;
        }
        OnTcpConnected(conn, time);
    };
    callback.OnDataReceived = [this](TcpConn* conn, const DateTime& time) {
        if (limiter && limiter->ReachQpsLimits(conn)) {
            conn->Shutdown();
            return;
        }
        this->OnTcpReceivedData(conn, time);
    };
    callback.OnDisconnected = [this](TcpConn* conn, const DateTime& time) {
        this->OnTcpDisconnected(conn, time);
    };

    // Start all components
    worker_group.Start(config.workgroup.workers,
                       InetAddress(config.acceptor.listen_ip, config.acceptor.listen_port),
                       callback,
                       ssl_ctx);
    thread_pool.Start(config.threadpool.workers, config.threadpool.pending_size);

    // handle signals
    struct sigaction action {};
    action.sa_handler = TcpServer::SignalHandler;
    sigaction(SIGTERM, &action, nullptr);
    sigaction(SIGINT, &action, nullptr);

    // monitor service status
    while (!stop_all_server.load()) {
        Monitor();
    }

    // stop all components here
    thread_pool.Stop();
    worker_group.Stop();
    LOG_INFO("Server is stopped")
}

void TcpServer::OnTcpConnected(TcpConn* conn, const jerry::DateTime& time) {
    // do nothing by default
    LOG_DEBUG("Client %s connected at %s",
              conn->GetPeerAddress().GetHost().c_str(),
              time.ToString().c_str())
}

void TcpServer::OnTcpDisconnected(TcpConn* conn, const jerry::DateTime& time) {
    // do nothing by default
    LOG_DEBUG("Client %s disconnected at %s",
              conn->GetPeerAddress().GetHost().c_str(),
              time.ToString().c_str())
}

void TcpServer::OnTcpReceivedData(TcpConn* conn, const jerry::DateTime& time) {
    // drop the data by default
    LOG_DEBUG("Server received %zu bytes from %s at %s",
              conn->GetRecvBuffer().ReadableBytes(),
              conn->GetPeerAddress().GetHost().c_str(),
              time.ToString().c_str())
    conn->GetRecvBuffer().DropAllBytes();
}

void TcpServer::SignalHandler(int sig) {
    LOG_WARN("SIGNAL %d received, server will exit soon...", sig)
    TcpServer::stop_all_server.store(true);
}

void TcpServer::SetTcpRateLimiter(TcpRateLimiter* limiter) {
    this->limiter = limiter;
}

void TcpServer::Monitor() {
    // TODO: Monitor the load of each component and do dynamic configuration adjustment
    std::this_thread::sleep_for(std::chrono::seconds(5));
    LOG_DEBUG("[Health Check] Total connections: %zu, Total timers: %zu, Pending tasks: %zu",
              GetTotalConns(),
              GetTotalTimers(),
              GetPendingTasks())
}


}  // namespace jerry::net