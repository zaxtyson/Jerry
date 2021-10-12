//
// Created by zaxtyson on 2021/9/20.
//

#include <unistd.h>
#include <net/Socket.h>
#include <utils/log/Logger.h>
#include <cassert>
#include <cerrno>
#include <cstring>

Socket::Socket() {
    sockFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd_ == -1) {
        LOG_FATAL("Created socket failed, %s", strerror(errno));
    }
    LOG_INFO("Created socket success, socket fd = %d", sockFd_);
}

Socket::~Socket() {
    if (sockFd_ > 0) {
        LOG_INFO("Socket closed, fd = %d", sockFd_);
        close(sockFd_);
    }
}

void Socket::bindAddress(const InetAddress &localAddress) {
    assert(sockFd_ > 0);
    localAddress_ = localAddress;
    if (::bind(sockFd_, localAddress_.getSockAddr(), localAddress_.getSockLen()) == -1) {
        LOG_FATAL("Bind %s failed, sockfd = %d", localAddress.getIpPort().c_str(), sockFd_);
    }
    LOG_INFO("Bind %s success, sockfd = %d", localAddress.getIpPort().c_str(), sockFd_);
}

void Socket::listen() {
    assert(sockFd_ > 0);
    // backlog 参数描述的是服务器端 TCP ESTABELLISHED 状态对应的全连接队列长度
    // 全连接队列长度 = min(backlog, 内核参数 net.core.somaxconn)，net.core.somaxconn 默认为 128
    // 半连接队列长度 = min(backlog, 内核参数 net.core.somaxconn，内核参数 tcp_max_syn_backlog)
    // 半连接队列长度由内核参数 tcp_max_syn_backlog 决定，
    // 当使用 SYN Cookie 时（就是内核参数 net.ipv4.tcp_syncookies = 1），这个参数无效
    // See:
    // https://jaminzhang.github.io/linux/understand-Linux-backlog-and-somaxconn-kernel-arguments/
    // https://stackoverflow.com/questions/62641621/what-is-the-difference-between-tcp-max-syn-backlog-and-somaxconn/62643129
    if (::listen(sockFd_, SOMAXCONN) < 0) {
        LOG_FATAL("Listen failed, sockfd = %d", sockFd_);
    }
}

int Socket::accept(InetAddress &peerAddress) {
    socklen_t len = peerAddress.getSockLen();
    int clientFd = ::accept4(sockFd_, peerAddress.getSockAddr(), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (clientFd < 0) {
        LOG_ERROR("Socket accept failed, sockfd = %d, %s", sockFd_, strerror(errno));
    }
    return clientFd;
}


int Socket::connect(const InetAddress &peerAddress) {
    return ::connect(sockFd_, peerAddress.getSockAddr(), peerAddress.getSockLen());
}


