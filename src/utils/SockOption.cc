//
// Created by zaxtyson on 2021/9/20.
//

#include "SockOption.h"
#include <fcntl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

void setTcpNoDelay(int fd) {
    // https://stackoverflow.com/questions/3761276/when-should-i-use-tcp-nodelay-and-when-tcp-cork
    // https://www.jianshu.com/p/ccafdeda0b95
    int opt = 1;
    ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
}

void setReuseAddr(int fd) {
    // https://stackoverflow.com/questions/14388706/how-do-so-reuseaddr-and-so-reuseport-differ
    int opt = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

void setReusePort(int fd) {
    // https://man7.org/linux/man-pages/man7/socket.7.html
    int opt = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
}

void setNonBlockAndCloseOnExec(int fd) {
    // set nonblock
    int flags = ::fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    ::fcntl(fd, F_SETFL, flags);

    // set close on exec
    flags = ::fcntl(fd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ::fcntl(fd, F_SETFD, flags);
}

void setKeepAlive(int fd, int idleSecs, int intervalSecs, int cnt) {
    // https://www.zhihu.com/question/40602902
    int opt = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
    // 设置详细配置
    ::setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idleSecs, sizeof(idleSecs));
    ::setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &intervalSecs, sizeof(intervalSecs));
    ::setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
}
