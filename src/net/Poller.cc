//
// Created by zaxtyson on 2022/3/12.
//

#include "Poller.h"
#include "Channel.h"
#include "logger/Logger.h"
#include "utils/FdHelper.h"

namespace jerry::net {

int Poller::GetFd() const {
    return fd;
}

Poller::Poller() {
    // create the poller
    fd = utils::CreateEpollFd();
    if (fd == -1) {
        LOG_FATAL("Create poller failed!")
    }
    LOG_DEBUG("Create poller success, fd = %d", fd)

    // create event_fd to wake up the poller
    int wakeup_fd = utils::CreateEventFd();
    if (wakeup_fd == -1) {
        LOG_FATAL("Create wakeup fd failed!")
    }
    LOG_DEBUG("Create wakeup fd success, fd = %d", wakeup_fd)
    ChannelCallback wakeup_callback;
    wakeup_callback.OnReadable = [wakeup_fd](const DateTime&) {
        uint64_t value;  // fetch the data to avoid busy-loop
        read(wakeup_fd, &value, sizeof(value));
    };
    wakeup_channel = new Channel(wakeup_fd);
    wakeup_channel->SetCallback(std::move(wakeup_callback));
    wakeup_channel->SetPoller(this);
    wakeup_channel->AddToPoller();
    wakeup_channel->ActivateReading();
}

Poller::~Poller() {
    wakeup_channel->RemoveFromPoller();
    delete wakeup_channel;
    close(fd);
    LOG_DEBUG("Poller closed, fd = %d", fd)
}

void Poller::AddChannel(Channel* channel) {
    int c_fd = channel->GetFd();
    epoll_event evt{};
    int ret = epoll_ctl(fd, EPOLL_CTL_ADD, c_fd, &evt);
    if (ret == -1) {
        LOG_ERROR("Failed to add channel %d to poller %d: %s", c_fd, fd, strerror(errno))
        return;
    }
    LOG_DEBUG("Add channel %d to poller %d success", c_fd, fd)
}

void Poller::RemoveChannel(Channel* channel) {
    // In kernel versions before 2.6.9, the EPOLL_CTL_DEL operation
    // required a non-null pointer in event, even though this argument
    // is ignored.  Since Linux 2.6.9, event can be specified as NULL
    // when using EPOLL_CTL_DEL.
    int c_fd = channel->GetFd();
    epoll_event evt{};
    int ret = epoll_ctl(fd, EPOLL_CTL_DEL, c_fd, &evt);
    if (ret == -1) {
        LOG_ERROR("Failed to Remove channel %d from poller %d", c_fd, fd)
        return;
    }
    LOG_DEBUG("Remove channel %d from poller %d success", c_fd, fd)
}

void Poller::UpdateChannel(Channel* channel) {
    int c_fd = channel->GetFd();
    epoll_event evt = channel->GetEpollEvent();
    int ret = epoll_ctl(fd, EPOLL_CTL_MOD, c_fd, &evt);
    if (ret == -1) {
        LOG_ERROR("Failed to update channel %d in poller %d: %s", c_fd, fd, strerror(errno))
        return;
    }
    // LOG_DEBUG("Update channel %d in poller %d success", c_fd, fd);
}

jerry::DateTime Poller::Poll(Poller::ChannelList& active_channels) {
    int fd_nums = epoll_wait(
        fd, active_events.data(), static_cast<int>(active_events.capacity()), kPollTimeout);
    DateTime now = DateTime::Now();
    if (fd_nums == -1) {
        LOG_ERROR("epoll_wait return -1, epfd = %d: %s", fd, strerror(errno))
        return now;
    }

    if (fd_nums > 0) {
        LOG_DEBUG("Epoll fd = %d return %d active fd(s)", fd, fd_nums)
    }

    for (int i = 0; i < fd_nums; i++) {
        auto* channel = reinterpret_cast<Channel*>(active_events[i].data.ptr);
        channel->SetTriggeredEvent(active_events[i].events);
        active_channels.emplace_back(channel);
    }
    return now;
}

void Poller::Wakeup() {
    uint64_t value = 1;  // counter + 1
    write(wakeup_channel->GetFd(), &value, sizeof(value));
}

}  // namespace jerry::net