//
// Created by zaxtyson on 2021/9/19.
//

#include <net/Channel.h>
#include <sys/epoll.h>
#include <utils/log/Logger.h>

// See https://man7.org/linux/man-pages/man2/epoll_ctl.2.html

// 测试用, 获取 Event 的类型
static std::string getEventTypeName(int event) {
    std::string ss;
    if (event & EPOLLIN) ss += "EPOLLIN ";
    if (event & EPOLLOUT) ss += "EPOLLOUT ";
    if (event & EPOLLERR) ss += "EPOLLERR ";
    if (event & EPOLLHUP) ss += "EPOLLHUP ";
    if (event & EPOLLRDHUP) ss += "EPOLLRDHUP ";
    return ss;
}

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

void Channel::handleEvent(Date date) {
    LOG_DEBUG("fd = %d, event = %s", fd_, getEventTypeName(revents_).c_str());
    if (revents_ & EPOLLHUP) {
        // 收到对端发来的 RST, 说明对方已经真的关闭了
        // 对方 close, 我们再 write, 第一次会成功, 然后立刻触发 EPOLLHUP
        // 后面再 write, 返回 -1, errno = EPIPE, 如果不捕获 SIGPIPE, 程序会退出
        if (closeCallback_) closeCallback_();
    } else if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        // 此时 fd 有数据可读, 可能是真的有数据, 也可能是对方调用 shutdown(SHUT_WR)/close(), 我们收到了 FIN
        // 当 epoll_ctl 关注了 EPOLLRDHUP, 收到 FIN 时才会返回该事件, 但是无法区分对方是 shutdown 还是 close
        // 收到 FIN, EPOLLIN 也会触发，read = 0, 为了简化我们统一用 read = 0 来判断对方关闭
        // See https://stackoverflow.com/questions/52976152/tcp-when-is-epollhup-generated
        if (readCallback_) readCallback_(date);
    } else if (revents_ & EPOLLOUT) {
        if (writeCallback_) writeCallback_();
    } else {
        if (errorCallback_) errorCallback_();
    }
}

Channel::~Channel() {
    if (fd_ > 0) {
        close(fd_);
    }
}
