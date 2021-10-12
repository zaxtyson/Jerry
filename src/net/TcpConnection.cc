//
// Created by zaxtyson on 2021/9/20.
//

#include <net/TcpConnection.h>
#include <cstring>
#include <utils/SockOption.h>
#include <utils/log/Logger.h>
#include <cassert>

TcpConnection::TcpConnection(EventLoop *loop, int fd, const InetAddress &localAddress, const InetAddress &peerAddress) :
        loop_(loop), localAddress_(localAddress), peerAddress_(peerAddress), bindChannel_() {
    // 设置我们关注的事件, 对应的事件触发时, 事件循环会调用对应的回调函数
    ::setNonBlockAndCloseOnExec(fd);
    ::setKeepAlive(fd);
    ::setTcpNoDelay(fd);
    bindChannel_.setFd(fd);
    bindChannel_.enableReading(); // 关注可读事件

    // 绑定对应的回调函数
    bindChannel_.setReadCallback([this](Date date) { handleRead(date); });
    bindChannel_.setWriteCallback([this] { handleWrite(); });
    bindChannel_.setCloseCallback([this] { handleClose(); });
    bindChannel_.setErrorCallback([this] { handleError(); });
}

void TcpConnection::shutdown() {
    // 调用时对方已经处于 shutdown(WR) 状态, 我们也 shutdown 优雅关闭
    // 此时会触发 EPOLLHUP
    // https://stackoverflow.com/questions/52976152/tcp-when-is-epollhup-generated
    // https://segmentfault.com/a/1190000020303015
    // https://mp.weixin.qq.com/s/Z0EqSihRaRbMscrZJl-zxQ

    auto thisPtr = shared_from_this();
    loop_->runInLoop([thisPtr] {
        if (::shutdown(thisPtr->getFd(), SHUT_WR) < 0) {
            LOG_ERROR("Socket shutdown write failed, sockfd = %d, %s", thisPtr->getFd(), strerror(errno));
        }
    });
}

void TcpConnection::handleRead(Date date) {
    // EPOLLIN, EPOLLPRI, EPOLLRDHUP
    int errnoBackup = 0;
    ssize_t n = bufferIn_.readFromFd(getFd(), &errnoBackup);
    if (n == 0) {
        // 客户端关闭连接, 不知道是那一种情况, 保险起见我们看成半关闭,
        // 应用层不能在 send, 但是 BufferOut 的数据我们还是尽量发出去
        LOG_INFO("Read 0 bytes, peer maybe close() or shutdown(WR)");
        isPeerShutdown_ = true;
        bindChannel_.disableReading(); // 对方已经不再写了, 我们不再关注可读事件
        loop_->updateChannel(&bindChannel_);
        trySendBufferRemains();
    } else if (n > 0) {
        // 正常读到数据, 通知上层处理
        if (recvMsgCallback_) recvMsgCallback_(shared_from_this(), bufferIn_, date);
    } else if (n < 0) {
        errno = errnoBackup;
        handleError();
    }
}

void TcpConnection::handleWrite() {
    LOG_DEBUG("%zu bytes waiting to send", bufferOut_.readableBytes());
    ssize_t n = write(getFd(), bufferOut_.readBegin(), bufferOut_.readableBytes());
    if (n < 0) {
        LOG_ERROR("TcpConnection write error, fd=%d, %s", getFd(), strerror(errno));
        if (errno == EPIPE || errno == ECONNRESET) {
            handleClose();
        }
        return;
    }

    bufferOut_.drop(n); // 已经放入内核缓冲区的数据扔掉
    LOG_DEBUG("Write %zu bytes to kernel send buffer, remains %zu bytes", n, bufferOut_.readableBytes());
    if (bufferOut_.readableBytes() == 0) {
        // 缓冲区发送完成，不再关注可写事件, 否则一直触发 EPOLLOUT
        bindChannel_.disableWriting();
        loop_->updateChannel(&bindChannel_);

        // 全部写入内核发送缓冲区了, 通知上层我们数据"发送"完成
        if (writeCompleteCallback_) writeCompleteCallback_(shared_from_this());

        // 如果对方是半关闭状态, 我们把 BufferOut 发完就算完成任务了
        if (isPeerShutdown_) {
            shutdown();
        }
    }

    // 没发完的数据等待下一次 EPOLLOUT
}

void TcpConnection::handleClose() {
    // EPOLLHUP, 对方发送 RST
    LOG_INFO("Peer has closed, address: %s", peerAddress_.getIpPort().c_str());
    isConnected_ = false; // 连接已经断掉了
    bindChannel_.disableAllEvent();
    loop_->updateChannel(&bindChannel_);

    // 调用连接关闭的回调
    if (closeCallback_) closeCallback_(shared_from_this());

    // 清理连接
    cleanupCallback_(shared_from_this());
}

void TcpConnection::handleError() {
    LOG_ERROR("Some thing error, peer address: %s, %s", peerAddress_.getIpPort().c_str(), strerror(errno));
    if (errorCallback_) errorCallback_(shared_from_this());
    handleClose();
}

TcpConnection::~TcpConnection() {
    // 从事件循环移除 Channel
    loop_->removeChannel(&bindChannel_);
    // 析构时 Channel 自动关闭了
}

void TcpConnection::send(const std::string &data) {
    assert(this != nullptr);
    if (!isConnected_) {
        LOG_DEBUG("TCP is not connected, cannot send data to client");
        return; // 连接已经断开
    }
    if (isPeerShutdown_) {
        LOG_DEBUG("Peer maybe shutdown, don't send data anymore");
        return; // 对方可能 close 或者 shutdown(WR), 应用层别再发数据了
    }

    auto thisPtr = shared_from_this();
    loop_->runInLoop([thisPtr, data] {
        thisPtr->sendInLoop(data);
    });
}

void TcpConnection::sendInLoop(const std::string &data) {
    bufferOut_.append(data);
    if (!bindChannel_.isWritable()) {
        bindChannel_.enableWriting();
        loop_->updateChannel(&bindChannel_);
    }
}

void TcpConnection::trySendBufferRemains() {
    if (bufferOut_.readableBytes() == 0) {
        handleClose();
    } else {
        handleWrite();
    }
}

