//
// Created by zaxtyson on 2021/9/20.
//

#include "SockOption.h"
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

namespace jerry::utils {

void SetTcpNoDelay(int fd) {
    // https://stackoverflow.com/questions/3761276/when-should-i-use-tcp-nodelay-and-when-tcp-cork
    // https://www.jianshu.com/p/ccafdeda0b95
    int opt = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
}

void SetReuseAddr(int fd) {
    // https://stackoverflow.com/questions/14388706/how-do-so-reuseaddr-and-so-reuseport-differ
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

void SetReusePort(int fd) {
    // https://man7.org/linux/man-pages/man7/socket.7.html
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
}

void SetNonBlockAndCloseOnExec(int fd) {
    // set nonblock
    int flags = fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);

    // set close on exec
    flags = fcntl(fd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    fcntl(fd, F_SETFD, flags);
}

void SetKeepAlive(int fd, int idle_secs, int interval_secs, int cnt) {
    // https://www.zhihu.com/question/40602902
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

    setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle_secs, sizeof(idle_secs));
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &interval_secs, sizeof(interval_secs));
    setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
}

}  // namespace jerry::utils