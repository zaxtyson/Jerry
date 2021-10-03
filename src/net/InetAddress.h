//
// Created by zaxtyson on 2021/9/19.
//

#ifndef JERRY_INETADDRESS_H
#define JERRY_INETADDRESS_H

#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * 封装网络地址类, 方便使用
 */
class InetAddress {
public:
    InetAddress() : addr_() {}

    explicit InetAddress(const std::string &ip, uint16_t port);

    explicit InetAddress(const sockaddr_in &addr) : addr_(addr) {}

    /**
     * 获取 IPv4 地址字符串
     * @return
     */
    std::string getIp() const;

    /**
     * 获取端口号
     * @return
     */
    uint16_t getPort() const;

    /**
     * 获取主机地址, ip:port
     * @return
     */
    std::string getIpPort() const;

    /**
     * 设置 Socket 地址
     * @param addr
     */
    void setSockAddr(const sockaddr_in &addr) { addr_ = addr; }

    /**
     * 获取 Socket 地址
     * @return
     */
    const sockaddr *getSockAddr() const { return reinterpret_cast<const sockaddr *>(&addr_); }

    sockaddr *getSockAddr() { return reinterpret_cast<sockaddr *>(&addr_); }

    /**
     * 获取 Socket Struct 长度
     * @return
     */
    socklen_t getSockLen() const { return sizeof(addr_); }


private:
    sockaddr_in addr_{};
};


#endif //JERRY_INETADDRESS_H
