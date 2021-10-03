//
// Created by zaxtyson on 2021/9/20.
//

#ifndef JERRY_SOCKET_H
#define JERRY_SOCKET_H

#include <utils/NonCopyable.h>
#include <net/InetAddress.h>
#include <utils/SockOption.h>

/**
 * 封装 Socket
 */
class Socket : NonCopyable {
public:
    Socket();

    explicit Socket(int sockFd) : sockFd_(sockFd) {};

    ~Socket();

    /**
     * 绑定本机地址
     * @param localAddress
     */
    void bindAddress(const InetAddress &localAddress);

    /**
     * 获取本机绑定的地址
     * @return
     */
    const InetAddress &getLocalAddress() const { return localAddress_; }

    /**
     * 监听本机地址, 全连接队列长度设置为 SOMAXCONN
     */
    void listen();

    /**
     * 等待对端连接
     * @return {客户端 fd, 客户端地址}
     */
    int accept(InetAddress &peerAddress);

    /**
     * 与给定的地址建立连接
     * @param peerAddress
     * @return
     */
    int connect(const InetAddress &peerAddress);

    /**
     * 获取 listen fd
     * @return
     */
    int getFd() const { return sockFd_; };


private:
    int sockFd_;
    InetAddress localAddress_;
};


#endif //JERRY_SOCKET_H
