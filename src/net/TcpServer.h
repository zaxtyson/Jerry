//
// Created by zaxtyson on 2022/3/12.
//

#ifndef JERRY_TCPSERVER_H
#define JERRY_TCPSERVER_H

#include "InetAddress.h"
#include "SslContext.h"
#include "TcpRateLimiter.h"
#include "ThreadPool.h"
#include "WorkerGroup.h"
#include "net/TimerQueue.h"  // for easy use

namespace jerry::net {

struct ServerConfig {
    struct {
        std::string listen_ip{"0.0.0.0"};
        uint16_t listen_port{80};
    } acceptor;

    struct {
        bool use_ssl{false};
        bool disable_old_ssl_version{false};
        bool enable_validation{false};
        std::string cert_file;
        std::string private_key_file;
    } ssl;

    struct {
        size_t workers{4};
    } workgroup;

    struct {
        size_t workers{4};
        size_t pending_size{0};
    } threadpool;
};

class TcpServer : NonCopyable {
  public:
    TcpServer() = default;
    virtual ~TcpServer();

  public:
    /**
     * Set a new configuration before Start the TcpServer, which specifies the
     * listening address, the ReadableBytes of WorkerGroup, TimerWorkerGroup and ThreadPool
     *
     * @param config server configuration
     */
    void Config(const ServerConfig& config);

    /**
     * Start the TcpServer
     * Main thread will be blocked here until we received signal `SIGTERM` or `SIGINT`
     * The server does NOT stop_loop immediately, but waits for all connections to close properly
     */
    void Serve();

    /**
     *  Monitor the load of Server
     */
    virtual void Monitor();

  public:
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

    /**
     * Get the number of pending tasks in thread pool
     * @return the number of pending tasks
     */
    size_t GetPendingTasks() const;

    /**
     * Set Tcp rate limiter to control the traffic
     * @param limiter tcp rate limiter
     */
    void SetTcpRateLimiter(TcpRateLimiter* limiter);

  public:
    // override the following functions to implement specific operations

    /**
     * This function will be called when a new client connect to server
     *
     * @param conn the connection between server and client
     * @param time the time of client connected with server
     */
    virtual void OnTcpConnected(TcpConn* conn, const DateTime& time);

    /**
     * This function will be called when a client disconnected
     * NOTE: if you set a pointer object to the context, you MUST release
     * the pointer object in this function, otherwise it will cause memory-leak!
     *
     * @param conn the connection of disconnected client, You can only get
     * information or manipulate the custom contexts of the connection,
     * not send or read any data from the buffer.
     * @param time the time of client disconnected
     */
    virtual void OnTcpDisconnected(TcpConn* conn, const DateTime& time);

    /**
     * This function will be called when any client sends data to server
     *
     * @param conn the connection of the client that sent the data
     * @param time the time of data receipt by the TCP/IP stack of linux kernel
     */
    virtual void OnTcpReceivedData(TcpConn* conn, const DateTime& time);

  private:
    static void SignalHandler(int sig);

  private:
    ServerConfig config{};
    WorkerGroup worker_group{};
    ThreadPool thread_pool{};
    SslContext* ssl_ctx{};
    TcpRateLimiter* limiter{};

  private:
    inline static std::atomic<bool> stop_all_server{false};
};
}  // namespace jerry::net


#endif  // JERRY_TCPSERVER_H
