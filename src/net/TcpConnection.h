//
// Created by zaxtyson on 2021/9/20.
//

#ifndef JERRY_TCPCONNECTION_H
#define JERRY_TCPCONNECTION_H

#include <net/EventLoop.h>
#include <utils/NonCopyable.h>
#include <net/InetAddress.h>
#include <net/Channel.h>
#include <utils/Date.h>
#include <net/MsgBuffer.h>
#include <thread>
#include <memory>
#include <functional>
#include <any>

class TcpConnection;

using spTcpConnection = std::shared_ptr<TcpConnection>;
using Callback = std::function<void(const spTcpConnection &)>;
using RecvMsgCallback = std::function<void(const spTcpConnection &, MsgBuffer &, Date)>;

/**
 * TCP 连接类
 */
class TcpConnection : NonCopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    TcpConnection(EventLoop *loop, int fd, const InetAddress &localAddress, const InetAddress &peerAddress);

    ~TcpConnection();

    /**
     * 获取本连接所绑定的 fd
     * @return
     */
    int getFd() const { return bindChannel_.getFd(); }

    /**
     * 获取处理本连接事件的 EventLoop
     * @return
     */
    EventLoop *getLoop() const { return loop_; }

    /**
     * 获取连接绑定的 Channel
     * @return
     */
    Channel *getChannel() const { return const_cast<Channel *>(&bindChannel_); }

    /**
     * 获取本端地址
     * @return
     */
    const InetAddress &getLocalAddress() const { return localAddress_; }

    /**
     * 获取对端地址
     * @return
     */
    const InetAddress &getPeerAddress() const { return peerAddress_; }

//    void setAsyncProcessing(bool on) { asyncProcessing_ = on; }

    /**
     * 并不是真的 close 掉, 而是关闭服务器写通道, 发送 FIN 告知对端我们要关闭了
     * 当对方调用 shutdown(SHUT_WR) 时, 我们会收到 EPOLLRDHUP
     */
    void shutdown();

    /**
     * 发送数据给对端
     * @param data
     */
    void send(const std::string &data);

    /**
     * 连接是否未断开
     * @return
     */
    bool isConnected() const { return isConnected_; }

public:
    // 设置回调函数
    void setRecvMsgCallback(const RecvMsgCallback &callback) { recvMsgCallback_ = callback; }

    void setWriteCompleteCallback(const Callback &callback) { writeCompleteCallback_ = callback; }

    void setCloseCallback(const Callback &callback) { closeCallback_ = callback; }

    void setErrorCallback(const Callback &callback) { errorCallback_ = callback; }

    void setCleanupCallback(const Callback &callback) { cleanupCallback_ = callback; }

public:
    /**
     * 设置连接的上下文环境, 生命周期由 TcpConnection 对象管理
     * @param context
     */
    void setContext(std::shared_ptr<void> context) { context_ = context; }

    void *getContext() { return context_.get(); }

private:
    // 事件触发时, 被 Channel 的回调
    void handleRead(Date);

    void handleWrite();

    void handleClose();

    void handleError();

    void trySendBufferRemains();

private:
    void sendInLoop(const std::string &data);

private:
    // 处理本连接事件的 EventLoop, 由主循环分配
    EventLoop *loop_;

    // 绑定的 Channel 对象, 生命周期与当前对象相同
    Channel bindChannel_;

    const InetAddress localAddress_;
    const InetAddress peerAddress_;
    bool isConnected_{true};
    bool isPeerShutdown_{false};

    // 发送/接收缓存
    MsgBuffer bufferIn_{};
    MsgBuffer bufferOut_{};

    // 连接绑定上下文对象, 析构时删除
    // https://stackoverflow.com/questions/39288891/why-is-shared-ptrvoid-legal-while-unique-ptrvoid-is-ill-formed
    std::shared_ptr<void> context_;
private:
    Callback writeCompleteCallback_;
    Callback closeCallback_;
    Callback errorCallback_;
    Callback cleanupCallback_;
    RecvMsgCallback recvMsgCallback_;
};


#endif //JERRY_TCPCONNECTION_H
