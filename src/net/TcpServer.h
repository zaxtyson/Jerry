//
// Created by zaxtyson on 2021/9/20.
//

#ifndef JERRY_TCPSERVER_H
#define JERRY_TCPSERVER_H

#include <utils/NonCopyable.h>
#include <net/TcpConnection.h>
#include <net/EventLoop.h>
#include <net/EventLoopThreadPool.h>
#include <net/InetAddress.h>
#include <net/Socket.h>
#include <map>
#include <mutex>
#include <fcntl.h>

class TcpServer : NonCopyable {
public:
    /**
     * 创建 TCP 服务器
     * @param mainLoop 用户在主线程创建的事件循环
     * @param bindAddress 服务器绑定的地址
     * @param workers 创建的 IO 线程数量
     */
    explicit TcpServer(EventLoop *mainLoop, const InetAddress &bindAddress, int workers);

    virtual ~TcpServer() = default;

    /**
     * 启动事件循环, 开始运行
     */
    void start();

    /**
     * save close
     */
    void stop();

    /**
     * 获取已经连接的用户数量
     * @return
     */
    int64_t getConnectNums() const { return connectionNums_; }

public:
    // 子类虚重写以下函数, 实现具体的业务功能
    virtual void onReceiveMessage(const spTcpConnection &conn, MsgBuffer &buffer, Date date) {}

    virtual void onNewConnection(const spTcpConnection &conn) {}

    virtual void onWriteComplete(const spTcpConnection &conn) {}

    virtual void onConnectionError(const spTcpConnection &conn) {}

    virtual void onConnectionClose(const spTcpConnection &conn) {}

    virtual void onServerClose() {}

private:
    void onNewConnection();

    void removeConnection(const spTcpConnection &conn);

    void onConnectionError();

private:
    EventLoop *mainLoop_;
    EventLoopThreadPool eventLoopThreadPool_;
    Socket acceptor_{};  // 在主事件循环中运行, 用于等待用户连接
    Channel acceptorChannel_{};
    int64_t connectionNums_{0};  // 当前连接的用户数量
    std::map<int, spTcpConnection> connectionList_{};
    std::mutex mtx_{};
    int idleFd_{open("/dev/null", O_RDONLY | O_CLOEXEC)};
};


#endif //JERRY_TCPSERVER_H
