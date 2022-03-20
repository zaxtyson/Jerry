//
// Created by zaxtyson on 2022/3/18.
//

#ifndef JERRY_ACCEPTOR_H
#define JERRY_ACCEPTOR_H

#include "Socket.h"
#include "TcpConn.h"

namespace jerry::net {

class IOWorker;

class Acceptor {
  public:
    explicit Acceptor(IOWorker* worker,
                      const InetAddress& listen_addr,
                      const TcpCallback& callback);
    ~Acceptor();

  private:
    void OnPeerConnect(const DateTime& time);

  private:
    int reserved_fd{};
    Socket* acceptor{};
    Channel* acceptor_channel{};
    IOWorker* worker{};          // the owner of this Acceptor
    TcpCallback tcp_callback{};  // callback for new TcpConn
};
}  // namespace jerry::net

#endif  // JERRY_ACCEPTOR_H
