//
// Created by zaxtyson on 2021/9/19.
//

#include <net/Poller.h>
#include <utils/Logger.h>
#include <cerrno>
#include <cstring>
#include <unistd.h>

Poller::Poller() {
    pollFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (pollFd_ == -1) {
        LOG_FATAL("Create poller failed, %s", strerror(errno));
        exit(1);
    }
}

Poller::~Poller() {
    LOG_INFO("Poller closed, fd = %d", pollFd_);
    close(pollFd_);
}

void Poller::addChannel(Channel *channel) {
    int fd = channel->getFd();
    epoll_event ev{};
    ev.events = channel->getFocusedEvent();
    ev.data.ptr = channel;

    if (epoll_ctl(pollFd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        LOG_ERROR("Failed to add channel(fd=%d) to poller(fd=%d)", fd, pollFd_);
        return;
    }
    channelNums_++;
    LOG_DEBUG("Add channel(fd=%d) to poller(fd=%d) success", fd, pollFd_);
}

void Poller::removeChannel(Channel *channel) {
    int fd = channel->getFd();
    epoll_event ev{};
    ev.events = channel->getFocusedEvent();
    ev.data.ptr = channel;

    if (epoll_ctl(pollFd_, EPOLL_CTL_DEL, fd, &ev) == -1) {
        LOG_ERROR("Failed to remove channel(fd=%d) from poller(fd=%d)", fd, pollFd_);
        return;
    }

    channelNums_--;
    LOG_DEBUG("Remove channel(fd=%d) from poller(fd=%d) success", fd, pollFd_);
}

void Poller::updateChannel(Channel *channel) {
    int fd = channel->getFd();
    epoll_event ev{};
    ev.events = channel->getFocusedEvent();
    ev.data.ptr = channel;

    if (epoll_ctl(pollFd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
        LOG_ERROR("Failed to update channel(fd=%d) in poller(fd=%d)", fd, pollFd_);
        return;
    }
//    LOG_DEBUG("Update channel(fd=%d) in poller(fd=%d) success", fd, pollFd_);
}

Date Poller::poll(ChannelList &activeChannelList) {
    int nfds = epoll_wait(pollFd_, eventList_.data(), static_cast<int>(eventList_.capacity()), POLLER_TIMEOUT);
    LOG_DEBUG("Epoll wait return %d fds", nfds);
    Date now = Date::now();
    if (nfds == -1) {
        LOG_ERROR("epoll_wait return -1, %s", strerror(errno));
        return now;
    }

    for (int i = 0; i < nfds; i++) {
        auto *channel = reinterpret_cast<Channel *>(eventList_[i].data.ptr);
        channel->setRealEvents(eventList_[i].events);  // 填写真实发生的事件
        activeChannelList.emplace_back(channel);
    }
    return now;
}

uint64_t Poller::getChannelNums() const {
    return channelNums_;
}
