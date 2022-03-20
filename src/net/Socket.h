//
// Created by zaxtyson on 2022/3/12.
//

#ifndef JERRY_SOCKET_H
#define JERRY_SOCKET_H

#include "InetAddress.h"
#include "utils/NonCopyable.h"

namespace jerry::net {

class Socket : NonCopyable {
  public:
    Socket();
    ~Socket();

    int GetFd() const;
    InetAddress GetLocalAddr() const;
    void Bind(const InetAddress& local_addr);
    void Listen();
    std::tuple<int, InetAddress> Accept();
    int Connect(const InetAddress& peer_addr);

  private:
    int fd{};
    InetAddress local_addr;
};
}  // namespace jerry::net


#endif  // JERRY_SOCKET_H
