//
// Created by zaxtyson on 2022/3/12.
//

#include "InetAddress.h"
#include <cassert>

namespace jerry::net {

InetAddress::InetAddress(std::string_view ip, uint16_t port, bool is_ipv6) {
    assert(!ip.empty());
    assert(port > 0 && port < 65535);
    if (!is_ipv6) {
        addr_v4.sin_family = AF_INET;
        addr_v4.sin_port = htons(port);
        [[maybe_unused]] int rc = inet_pton(AF_INET, ip.data(), &addr_v4.sin_addr);
        assert(rc > 0);
    }
    // TODO: Supported ip_v6
}

uint16_t InetAddress::GetPort() const {
    return ntohs(addr_v4.sin_port);
}

std::string InetAddress::GetIp() const {
    char buf[64]{};
    inet_ntop(AF_INET, &addr_v4.sin_addr, buf, 64);
    return buf;
}

std::string InetAddress::GetHost() const {
    return std::string(GetIp() + ":" + std::to_string(GetPort()));
}

void InetAddress::SetSockAddr(const sockaddr_in& addr) {
    this->addr_v4 = addr;
}

const sockaddr* InetAddress::GetSockAddr() const {
    return reinterpret_cast<const sockaddr*>(&addr_v4);
}

sockaddr* InetAddress::GetSockAddr() {
    return reinterpret_cast<sockaddr*>(&addr_v4);
}

socklen_t InetAddress::GetSockLength() const {
    return sizeof(addr_v4);
}

}  // namespace jerry::net