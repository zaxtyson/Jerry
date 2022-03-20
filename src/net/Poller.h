//
// Created by zaxtyson on 2022/3/12.
//

#ifndef JERRY_POLLER_H
#define JERRY_POLLER_H

#include <sys/epoll.h>  // epoll_event
#include <vector>
#include "utils/DateTime.h"
#include "utils/NonCopyable.h"

namespace jerry::net {

class Channel;  // forwarding declaration to avoid cross-referencing

class Poller : NonCopyable {
  public:
    using EventList = std::vector<epoll_event>;
    using ChannelList = std::vector<Channel*>;

  public:
    Poller();
    ~Poller();

    int GetFd() const;
    void Wakeup();
    DateTime Poll(ChannelList& active_channels);
    void AddChannel(Channel* channel);
    void RemoveChannel(Channel* channel);
    void UpdateChannel(Channel* channel);

  private:
    int fd{};                 // poller fd
    Channel* wakeup_channel;  // use event_fd to wakeup epoll
    EventList active_events{kEventListMaxSize};

  public:
    static const int kPollTimeout{-1};  // ms, -1 block until poll return
    static const int kEventListMaxSize{4096};
};
}  // namespace jerry::net


#endif  // JERRY_POLLER_H
