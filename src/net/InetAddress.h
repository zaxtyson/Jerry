//
// Created by zaxtyson on 2022/3/12.
//

#ifndef JERRY_INETADDRESS_H
#define JERRY_INETADDRESS_H

#include <arpa/inet.h>
#include <string>
#include <string_view>

namespace jerry::net {
class InetAddress {
  public:
    InetAddress() = default;
    ~InetAddress() = default;

    explicit InetAddress(std::string_view ip, uint16_t port, bool is_ipv6 = false);

    uint16_t GetPort() const;
    std::string GetIp() const;
    std::string GetHost() const;

    void SetSockAddr(const sockaddr_in& addr);
    const sockaddr* GetSockAddr() const;
    sockaddr* GetSockAddr();
    socklen_t GetSockLength() const;

  private:
    sockaddr_in addr_v4{};
};
}  // namespace jerry::net


#endif  // JERRY_INETADDRESS_H
