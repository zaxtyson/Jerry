//
// Created by zaxtyson on 2022/3/12.
//

#ifndef JERRY_CHANNEL_H
#define JERRY_CHANNEL_H

#include <sys/epoll.h>
#include <unistd.h>
#include <cstdint>
#include <functional>
#include "utils/DateTime.h"
#include "utils/NonCopyable.h"

namespace jerry::net {

class Poller;  // forwarding declaration to avoid cross-referencing

struct ChannelCallback {
    using Type = std::function<void(const DateTime& time)>;

    Type OnWritable = nullptr;
    Type OnReadable = nullptr;
    Type OnPeerClose = nullptr;
    Type OnError = nullptr;
};

enum class EpollEvents : uint32_t {
    kNoneEvent = 0,
    kReadEvent = EPOLLIN | EPOLLPRI,
    kWriteEvent = EPOLLOUT,
    kCloseEvent = EPOLLHUP
};

class Channel : NonCopyable {
  public:
    explicit Channel(int fd);
    ~Channel();

  public:
    int GetFd() const;
    epoll_event GetEpollEvent();
    void SetPoller(Poller* poller);
    void SetTriggeredEvent(uint32_t evt);
    void SetCallback(ChannelCallback&& callback);
    ChannelCallback& GetCallback();

  public:
    bool IsActivateReading();
    bool IsActivateWriting();
    void ActivateReading();
    void DeactivateReading();
    void ActivateWriting();
    void DeactivateWriting();
    void ActivateRW();
    void DeactivateRW();
    void AddToPoller();

  public:
    void RemoveFromPoller();
    void HandleEvent(const DateTime& time);

  private:
    int fd{};
    Poller* poller{};             // the poller which owns the current channel
    uint32_t events{};            // the events we interested in
    uint32_t triggered_events{};  // the events really happened on this fd
    ChannelCallback callback;
};

}  // namespace jerry::net

#endif  // JERRY_CHANNEL_H
