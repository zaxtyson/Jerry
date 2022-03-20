//
// Created by zaxtyson on 2022/3/12.
//

#include "TcpConn.h"
#include <cassert>
#include <cstring>
#include "Channel.h"
#include "IOWorker.h"
#include "TimerQueue.h"
#include "logger/Logger.h"

namespace jerry::net {

TcpConn::TcpConn(int fd, const InetAddress& local_addr, const InetAddress& peer_addr)
    : channel{new Channel(fd)}, local_addr{local_addr}, peer_addr{peer_addr} {}

TcpConn::~TcpConn() {
    LOG_DEBUG("TcpConn destroy, peer addr: %s", peer_addr.GetHost().data())
    delete channel;
}

Channel* TcpConn::GetChannel() const {
    return channel;
}

IOWorker* TcpConn::GetIOWorker() const {
    return worker;
}

void TcpConn::SetIOWorker(IOWorker* worker) {
    this->worker = worker;
    this->channel->SetPoller(worker->GetPoller());
}

TcpState TcpConn::ExchangeState(TcpState state) {
    return this->state.exchange(state);
}

void TcpConn::RemoveFromIOWorker() {
    // MUST pass a shared_ptr here!
    // the TcpConn will destroy(immediately or with a delay) after the function below called
    // it will cause `heap-use-after-free` error if we pass a raw pointer
    // we should ensure that the TcpConn stays alive until the cleanup is complete
    worker->RemoveTcpConn(GetSharedPtr());
}

void TcpConn::SetCallback(const TcpCallback& callback) {
    this->callback = callback;

    // set callback for channel
    ChannelCallback channel_callback;

    // the lifecycle of TcpConn is always longer than its channel
    // thus, we don't bind a shared_ptr in callback
    channel_callback.OnReadable = [conn = this](const DateTime& time) {
        if (!conn->IsConnected()) {
            return;  // this connection has been closed
        }

        ssize_t n = conn->recv_buffer.ReadBytesFromFd(conn->GetChannel()->GetFd());
        if (n <= 0) {  // peer disconnected when we read, force close it
            conn->ForceClose();
            return;
        }

        // everything ok
        if (conn->callback.OnDataReceived) {
            conn->callback.OnDataReceived(conn, time);
        }
    };

    // on channel writable, send the data in send_buffer
    channel_callback.OnWritable = [conn = this]([[maybe_unused]] const DateTime& time) {
        if (!conn->IsConnected()) {
            LOG_DEBUG("Channel fd = %d is not writeable for tcp connection is shutdown/closed",
                      conn->GetChannel()->GetFd())
            return;
        }

        int fd = conn->channel->GetFd();
        ssize_t n = conn->send_buffer.WriteBytesToFd(fd);

        if (n < 0) {
            LOG_ERROR("Write data to fd %d error, %s, peer addr: %s",
                      fd,
                      strerror(errno),
                      conn->peer_addr.GetHost().c_str())
            if (errno == EPIPE || errno == ECONNRESET) {  // peer disconnected
                if (conn->callback.OnDisconnected) {
                    conn->callback.OnDisconnected(conn, DateTime::Now());
                }
                return;
            }
        }

        conn->send_buffer.DropBytes(n);  // drop the data already sent
        LOG_DEBUG("Write %zu bytes to fd = %d, remains %zu bytes",
                  n,
                  fd,
                  conn->send_buffer.ReadableBytes())

        // if no data to send, deactivate writing to prevent busy-loop(LT mode)
        if (conn->send_buffer.ReadableBytes() == 0) {
            conn->channel->DeactivateWriting();
        }
    };

    channel_callback.OnPeerClose = [conn = this](const DateTime& time) {
        conn->ExchangeState(TcpState::kDisconnected);
        conn->channel->RemoveFromPoller();

        // TcpConn object may be destroyed in an uncertain future, so we release the buffer
        // immediately after peer closed to prevent the garbage data from surviving too long
        conn->send_buffer.Release();
        conn->recv_buffer.Release();

        if (conn->callback.OnDisconnected) {
            conn->callback.OnDisconnected(conn, time);
        }
        // when the last shared_ptr of this TcpConn be removed, this TcpConn will be destroyed
        // maybe someone else owns the copy of shared_ptr(such as timer task)
        conn->RemoveFromIOWorker();
    };

    // if an error occurs, just close the connection
    channel_callback.OnError = channel_callback.OnPeerClose;

    channel->SetCallback(std::move(channel_callback));
}

bool TcpConn::IsConnected() const {
    return state.load() == TcpState::kConnected;
}

void TcpConn::Send(std::string_view data) {
    if (!IsConnected()) {
        LOG_DEBUG("The TcpConn disconnected, can't send any data, peer addr: %s",
                  peer_addr.GetHost().data())
        return;
    }
    send_buffer.Append(data);
    GetChannel()->ActivateWriting();
}

void TcpConn::Send(const BaseBuffer::value_type* data, size_t len) {
    Send({data, len});
}

void TcpConn::Send(BaseBuffer::const_iterator begin, size_t len) {
    Send({begin.base(), len});
}

void TcpConn::Send(BaseBuffer::const_iterator begin, BaseBuffer::const_iterator end) {
    Send({begin.base(), static_cast<size_t>(end - begin)});
}

void TcpConn::Shutdown() {
    // forbid user call `Send` after shutdown
    ExchangeState(TcpState::kShutdown);

    // no longer interest in writing events, no data will be writen to kernel buffer
    channel->DeactivateWriting();

    // send FIN to client, only shutdown the writing direction,
    // but server can still read from client
    auto rc = shutdown(channel->GetFd(), SHUT_WR);
    if (rc != 0) {
        LOG_WARN("Shutdown fd = %d failed: %s", channel->GetFd(), strerror(errno))
        ForceClose();
    }

    // client will call shutdown/close to End this connection after received FIN,
    // then the channel's callback `OnPeerClose` will be called
}

void TcpConn::ForceClose() {
    LOG_DEBUG("Close the TcpConn, channel fd = %d, peer addr: %s",
              channel->GetFd(),
              GetPeerAddress().GetHost().data())
    assert(channel->GetCallback().OnPeerClose);
    channel->GetCallback().OnPeerClose(DateTime::Now());
}

void TcpConn::SetContext(std::string_view key, const std::any& value) {
    context.Set(key, value);
}

const InetAddress& TcpConn::GetLocalAddress() const {
    return local_addr;
}

const InetAddress& TcpConn::GetPeerAddress() const {
    return peer_addr;
}

BaseBuffer& TcpConn::GetRecvBuffer() {
    return recv_buffer;
}

BaseBuffer& TcpConn::GetSendBuffer() {
    return send_buffer;
}

TimerQueue* TcpConn::GetTimerQueue() const {
    return GetIOWorker()->GetTimerQueue();
}

void TcpConn::SetDecoder(const std::any& decoder) {
    this->decoder = decoder;
}

std::shared_ptr<TcpConn> TcpConn::GetSharedPtr() {
    return shared_from_this();
}

bool TcpConnSet::Empty() const {
    LockGuard lock(mtx);
    return conn_set.empty();
}

void TcpConnSet::Add(TcpConn* conn) {
    LockGuard lock(mtx);
    conn_set.emplace(conn);
}

void TcpConnSet::Add(const TcpConnPtr& conn) {
    LockGuard lock(mtx);
    conn_set.emplace(conn);
}

void TcpConnSet::Remove(const TcpConnPtr& conn) {
    // Don't use LockGuard here, it will cause deadlock!
    // `Remove` will be called in `LockForeach`
    // we can use std::recursive_mutex in more complex situations
    conn_set.erase(conn);
}

void TcpConnSet::LockForEach(std::function<void(const TcpConnPtr&)>&& func) const {
    LockGuard lock(mtx);
    for (auto& conn : conn_set) {
        func(conn);
    }
}

size_t TcpConnSet::Size() const {
    LockGuard lock(mtx);
    return conn_set.size();
}

std::optional<std::any> TcpContext::Get(std::string_view key) const {
    LockGuard lock(mtx);
    if (context.find(key.data()) == context.end()) {
        return std::nullopt;
    }
    return context.at(key.data());
}

void TcpContext::Set(std::string_view key, const std::any& value) {
    LockGuard lock(mtx);
    context.emplace(key, value);
}

}  // namespace jerry::net