//
// Created by zaxtyson on 2021/9/19.
//

#include <cstring>
#include <net/InetAddress.h>


InetAddress::InetAddress(const std::string &ip, uint16_t port) {
    if (port <= 0 || port > 65535) port = 80;
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    if (!ip.empty()) {
        addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    } else {
        addr_.sin_addr.s_addr = inet_addr("0");
    }
}

std::string InetAddress::getIp() const {
    char buf[64] = {0};
    inet_ntop(AF_INET, &addr_.sin_addr, buf, 64);
    return buf;
}

uint16_t InetAddress::getPort() const {
    return ntohs(addr_.sin_port);
}

std::string InetAddress::getIpPort() const {
    return std::string(getIp() + ":" + std::to_string(getPort()));
}
