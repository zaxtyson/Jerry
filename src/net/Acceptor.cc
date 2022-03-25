//
// Created by zaxtyson on 2022/3/18.
//

#include "Acceptor.h"
#include <fcntl.h>  // O_RDONLY, O_CLOEXEC
#include <cassert>
#include <cstring>  // strerror
#include "Channel.h"
#include "IOWorker.h"
#include "Poller.h"
#include "logger/Logger.h"
#include "utils/SockOption.h"

namespace jerry::net {

Acceptor::Acceptor(IOWorker* worker, const InetAddress& listen_addr, const TcpCallback& callback) {
    this->worker = worker;
    this->acceptor = new Socket();
    this->tcp_callback = callback;

    // reserve an idle fd to recovery from fd insufficient
    reserved_fd = open("/dev/null", O_RDONLY | O_CLOEXEC);
    assert(reserved_fd > 0);

    // to enable kernel's load-balance feature, we set `SO_REUSEPORT` before
    // listen, it requires kernel version >= 3.9
    int acceptor_fd = acceptor->GetFd();
    utils::SetNonBlockAndCloseOnExec(acceptor_fd);
    utils::SetReuseAddr(acceptor_fd);
    utils::SetReusePort(acceptor_fd);  // must be set
    acceptor->Bind(listen_addr);
    acceptor->Listen();

    // bind fd with channel
    acceptor_channel = new Channel(acceptor_fd);
    acceptor_channel->SetPoller(worker->GetPoller());

    // bind callback to handle coming client, we only interest in the READ event
    ChannelCallback acceptor_callback;
    acceptor_callback.OnReadable = [this](const DateTime& time) { this->OnPeerConnect(time); };
    acceptor_channel->SetCallback(std::move(acceptor_callback));

    // add channel to poller, wait for clients connection
    acceptor_channel->AddToPoller();
    acceptor_channel->ActivateReading();
}

Acceptor::~Acceptor() {
    delete acceptor;
    delete acceptor_channel;
    close(reserved_fd);
}

void Acceptor::OnPeerConnect(const DateTime& time) {
    auto [client_fd, peer_addr] = acceptor->Accept();

    // fd has exhausted
    if (client_fd < 0) {
        if (errno == EMFILE) {
            close(reserved_fd);
            std::tie(reserved_fd, peer_addr) = acceptor->Accept();
            close(reserved_fd);
            reserved_fd = open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
        LOG_WARN("Accept new client failed: %s", strerror(errno))
        return;
    }

    // create TcpConn for new client, bind with user specify tcp_callback
    // default state is `State::kConnected`
    LOG_DEBUG("New client connected: %s, fd = %d", peer_addr.GetHost().c_str(), client_fd)
    auto* conn = new TcpConn(client_fd, acceptor->GetLocalAddr(), peer_addr);
    conn->SetIOWorker(worker);        // bind TcpConn with IOWorker
    conn->SetCallback(tcp_callback);  // set tcp callback

    // store the shared_ptr of conn in IOWorker to extend its lifecycle
    worker->AddTcpConn(conn);

    // we interest in the read event
    conn->GetChannel()->AddToPoller();
    conn->GetChannel()->ActivateReading();

    // after above, we call OnConnected to notify caller that a new client connected
    if (tcp_callback.OnConnected) {
        tcp_callback.OnConnected(conn, time);
    }

    // try creating SSL on this connection
    conn->InitSsl();
}

}  // namespace jerry::net