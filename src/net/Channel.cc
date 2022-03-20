//
// Created by zaxtyson on 2022/3/12.
//


#include "Channel.h"
#include <logger/Logger.h>
#include <sys/epoll.h>
#include <map>
#include "Poller.h"
#include "utils/FdHelper.h"

namespace jerry::net {

Channel::Channel(int fd) : fd{fd} {}

Channel::~Channel() {
    jerry::utils::Close(fd);
}

void Channel::SetPoller(Poller* poller) {
    this->poller = poller;
}

void Channel::SetCallback(ChannelCallback&& callback) {
    this->callback = callback;
}

bool Channel::IsActivateReading() {
    return events & static_cast<uint32_t>(EpollEvents::kReadEvent);
}

bool Channel::IsActivateWriting() {
    return events & static_cast<uint32_t>(EpollEvents::kWriteEvent);
}

void Channel::ActivateReading() {
    if (!IsActivateReading()) {
        events |= static_cast<uint32_t>(EpollEvents::kReadEvent);
        poller->UpdateChannel(this);
    }
}

void Channel::DeactivateReading() {
    if (IsActivateReading()) {
        events &= ~static_cast<uint32_t>(EpollEvents::kReadEvent);
        poller->UpdateChannel(this);
    }
}

void Channel::ActivateWriting() {
    if (!IsActivateWriting()) {
        events |= static_cast<uint32_t>(EpollEvents::kWriteEvent);
        poller->UpdateChannel(this);
    }
}

void Channel::DeactivateWriting() {
    if (IsActivateWriting()) {
        events &= ~static_cast<uint32_t>(EpollEvents::kWriteEvent);
        poller->UpdateChannel(this);
    }
}

void Channel::ActivateRW() {
    if (!IsActivateReading() || !IsActivateWriting()) {
        events |= static_cast<uint32_t>(EpollEvents::kReadEvent);
        events |= static_cast<uint32_t>(EpollEvents::kWriteEvent);
        poller->UpdateChannel(this);
    }
}

void Channel::DeactivateRW() {
    if (IsActivateReading() || IsActivateWriting()) {
        events = static_cast<uint32_t>(EpollEvents::kNoneEvent);
        poller->UpdateChannel(this);
    }
}

void Channel::AddToPoller() {
    poller->AddChannel(this);
}

void Channel::RemoveFromPoller() {
    poller->RemoveChannel(this);
}

int Channel::GetFd() const {
    return fd;
}

ChannelCallback& Channel::GetCallback() {
    return callback;
}

epoll_event Channel::GetEpollEvent() {
    epoll_event evt{};
    evt.events = events;
    evt.data.ptr = this;
    return evt;
}

void Channel::SetTriggeredEvent(uint32_t evt) {
    triggered_events = evt;
}

static std::string_view GetEventTypeName(uint32_t event) {
    static std::map<uint32_t, std::string> evt_map;  // cache known names
    if (evt_map.find(event) == evt_map.end()) {
        std::string evt_name;
        if (event & EPOLLIN) evt_name += "EPOLLIN ";
        if (event & EPOLLPRI) evt_name += "EPOLLPRI ";
        if (event & EPOLLOUT) evt_name += "EPOLLOUT ";
        if (event & EPOLLERR) evt_name += "EPOLLERR ";
        if (event & EPOLLHUP) evt_name += "EPOLLHUP ";
        if (event & EPOLLRDHUP) evt_name += "EPOLLRDHUP ";
        if (evt_name.back() == ' ') evt_name.erase(evt_name.end() - 1);
        evt_map.emplace(event, evt_name);
    }
    return evt_map.at(event);
}

void Channel::HandleEvent(const jerry::DateTime& time) {
    /**
     * See: https://stackoverflow.com/questions/52976152/tcp-when-is-epollhup-generated
     *
     * Doing shutdown(SHUT_WR) sends a FIN and marks the socket with SEND_SHUTDOWN.
     * Doing shutdown(SHUT_RD) sends nothing and marks the socket with RCV_SHUTDOWN.
     * Receiving a FIN marks the socket with RCV_SHUTDOWN.
     *
     * If the socket is marked with SEND_SHUTDOWN and RCV_SHUTDOWN, poll will return EPOLLHUP.
     * If the socket is marked with RCV_SHUTDOWN, poll will return EPOLLRDHUP.
     */

    // poll will return EPOLLRDHUP only when we set EPOLLRDHUP in interest evens
    // for convenience, we use `read -> 0` to check if peer has closed rather than EPOLLRDHUP
    LOG_DEBUG("channel fd = %d, event = %s", fd, GetEventTypeName(triggered_events).data());

    if (triggered_events & static_cast<uint32_t>(EpollEvents::kCloseEvent)) {
        if (callback.OnPeerClose) {
            callback.OnPeerClose(time);
        }
    } else if (triggered_events & static_cast<uint32_t>(EpollEvents::kReadEvent)) {
        if (callback.OnReadable) {
            callback.OnReadable(time);
        }
    } else if (triggered_events & static_cast<uint32_t>(EpollEvents::kWriteEvent)) {
        if (callback.OnWritable) {
            callback.OnWritable(time);
        }
    } else {
        if (callback.OnError) {
            callback.OnError(time);
        }
    }
}

}  // namespace jerry::net