//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_POLLER_H
#define JERRY_POLLER_H

#include <utils/NonCopyable.h>
#include <sys/epoll.h>
#include <net/Channel.h>

// 一些常量
constexpr int EVENT_LIST_MAX = 4096; // epoll 每次返回的事件数量
constexpr int POLLER_TIMEOUT = -1; // epoll_wait 超时

/**
 * 不同的平台使用不同的 Poller
 * 为了简单起见, 我们使用 Linux 下流行的 epoll
 */
class Poller {
public:
    using ChannelList = std::vector<Channel *>;
    using EventList = std::vector<epoll_event>;
public:
    Poller();

    ~Poller();

    /**
     * 获取 Poller 文件描述符
     * @return
     */
    int getFd() const { return pollFd_; };

    /**
     * 添加一个 Channel 到 Poller
     * @param channel
     */
    void addChannel(Channel *channel);

    /**
     * 从 Poller 中移除一个 Channel
     * @param channel
     */
    void removeChannel(Channel *channel);

    /**
     * 更新 Channel 关注的事件
     * @param channel
     */
    void updateChannel(Channel *channel);

    /**
     * 获取 epoll_wait 返回的活跃事件
     * @param activeChannelList 活跃的 Channel 将倍填写到这里
     * @return 事件发生的时间
     */
    Date poll(ChannelList &activeChannelList);

    /**
     * 获取当前 Poller 中管理的 Channel 数量
     * @return
     */
    uint64_t getChannelNums() const;

private:
    int pollFd_{-1};
    uint64_t channelNums_{0};
    EventList eventList_{EVENT_LIST_MAX};
};


#endif //JERRY_POLLER_H
