//
// Created by zaxtyson on 2022/3/12.
//

#include "Socket.h"
#include <cassert>
#include "logger/Logger.h"
#include "utils/FdHelper.h"

namespace jerry::net {

Socket::Socket() {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        LOG_FATAL("Created socket failed")
    }
    LOG_INFO("Created socket success, socket fd = %d", fd)
}

Socket::~Socket() {
    jerry::utils::Close(fd);
}

int Socket::GetFd() const {
    return fd;
}

void Socket::Bind(const InetAddress& local_addr) {
    assert(fd > 0);
    this->local_addr = local_addr;
    int ret = bind(fd, local_addr.GetSockAddr(), local_addr.GetSockLength());
    if (ret == -1) {
        LOG_FATAL("Socket fd = %d bind [%s] failed", fd, local_addr.GetHost().data())
    }
    LOG_INFO("Socket fd = %d bind [%s] success", fd, local_addr.GetHost().data())
}

void Socket::Listen() {
    assert(fd > 0);
    // https://stackoverflow.com/questions/62641621/what-is-the-difference-between-tcp-max-syn-backlog-and-somaxconn/62643129
    if (listen(fd, SOMAXCONN) < 0) {
        LOG_FATAL("Listen failed, sockfd = %d", fd)
    }
}

std::tuple<int, InetAddress> Socket::Accept() {
    InetAddress peer_addr;
    socklen_t len = peer_addr.GetSockLength();
    int client_fd = accept4(fd, peer_addr.GetSockAddr(), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (client_fd < 0) {
        LOG_ERROR("Socket accept failed, sockfd = %d, %s", fd, strerror(errno))
    }
    return {client_fd, peer_addr};
}

int Socket::Connect(const InetAddress& peer_addr) {
    // for client, to implement...
    return connect(fd, peer_addr.GetSockAddr(), peer_addr.GetSockLength());
}

InetAddress Socket::GetLocalAddr() const {
    return local_addr;
}

}  // namespace jerry::net