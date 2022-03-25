//
// Created by zaxtyson on 2022/3/12.
//

#include "TcpConn.h"
#include <cassert>
#include <cstring>
#include "Channel.h"
#include "IOWorker.h"
#include "SslContext.h"
#include "TimerQueue.h"
#include "logger/Logger.h"

namespace jerry::net {

TcpConn::TcpConn(int fd, const InetAddress& local_addr, const InetAddress& peer_addr)
    : channel{new Channel(fd)}, local_addr{local_addr}, peer_addr{peer_addr} {}

TcpConn::~TcpConn() {
    LOG_DEBUG("TcpConn destroy, peer addr: %s", peer_addr.GetHost().data())
    delete channel;
    if (IsEncrypted()) {
        SSL_free(ssl);
    }
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

TcpConn::State TcpConn::ExchangeState(State state) {
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

    // the lifecycle of TcpConn is always longer than its channel
    // thus, we don't bind a shared_ptr in callback
    ChannelCallback channel_callback;
    channel_callback.OnReadable = [this](const DateTime& time) { this->OnChannelReadable(time); };
    channel_callback.OnWritable = [this](const DateTime& time) { this->OnChannelWritable(time); };
    channel_callback.OnPeerClose = [this](const DateTime& time) { this->OnChannelClose(time); };
    channel_callback.OnError = channel_callback.OnPeerClose;
    channel->SetCallback(std::move(channel_callback));
}

bool TcpConn::IsConnected() const {
    return state.load() == State::kConnected;
}

bool TcpConn::IsEncrypted() const {
    return worker->GetSslContext() != nullptr;
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
    ExchangeState(State::kShutdown);

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

void TcpConn::SetStreamCodec(const std::any& codec) {
    this->codec = codec;
}

std::shared_ptr<TcpConn> TcpConn::GetSharedPtr() {
    return shared_from_this();
}

void TcpConn::InitSsl() {
    if (IsEncrypted()) {
#ifdef USE_OPENSSL
        ssl = SSL_new(worker->GetSslContext()->Get());
        [[maybe_unused]] int rc = SSL_set_fd(ssl, GetChannel()->GetFd());
        assert(rc == 1);

        // for transparent negotiation
        SSL_set_accept_state(ssl);
        LOG_INFO("TcpConn [%s] is encrypted by OpenSSL", peer_addr.GetHost().data())
#else
        LOG_WARN("Your system has not installed openssl!")
#endif
    }
}

void TcpConn::OnChannelReadable(const DateTime& time) {
    // we can call `SSL_do_handshake(ssl)` here to do handshake manually
    // State::kExceptHandshake => SSL_do_handshake => State::kConnected

    if (!IsConnected()) {
        LOG_DEBUG("Channel fd = %d is not readable for tcp connection is shutdown/closed",
                  GetChannel()->GetFd())
        return;  // this connection has been closed
    }

    if (IsEncrypted()) {
#ifndef USE_OPENSSL
        LOG_FATAL("Your system has not installed openssl!")
#else
        ssize_t n = recv_buffer.ReadBytesFromSsl(ssl);
        if (n == 0) {
            Shutdown();
            return;
        }
        if (n < 0) {
            int rc = SSL_get_error(ssl, static_cast<int>(n));
            if (rc == SSL_ERROR_WANT_READ) {
                return;  // wait next time
            } else {
                char err_msg[4096];
                ERR_error_string_n(ERR_get_error(), err_msg, sizeof(err_msg));
                LOG_WARN("SSL read error: %u, %s", rc, err_msg)
                ForceClose();
                return;
            }
        }
#endif
    } else {
        ssize_t n = recv_buffer.ReadBytesFromFd(GetChannel()->GetFd());
        if (n <= 0) {  // peer disconnected when we read, force close it
            ForceClose();
            return;
        }
    }

    // everything ok
    if (callback.OnDataReceived) {
        callback.OnDataReceived(this, time);
    }
}

void TcpConn::OnChannelWritable(const DateTime& time) {
    if (!IsConnected()) {
        LOG_DEBUG("Channel fd = %d is not writeable for tcp connection is shutdown/closed",
                  GetChannel()->GetFd())
        return;
    }

    ssize_t n;
    if (IsEncrypted()) {
#ifndef USE_OPENSSL
        LOG_FATAL("Your system has not installed openssl!")
#else
        n = send_buffer.WriteBytesToSsl(ssl);
        if (n <= 0) {
            int rc = SSL_get_error(ssl, static_cast<int>(n));
            char err_msg[4096];
            ERR_error_string_n(ERR_get_error(), err_msg, sizeof(err_msg));
            LOG_WARN("SSL write error: %d, %s", rc, err_msg)
            ForceClose();
        }
#endif
    } else {
        n = send_buffer.WriteBytesToFd(channel->GetFd());
        if (n < 0) {
            LOG_ERROR("Write data to fd %d error, %s, peer addr: %s",
                      channel->GetFd(),
                      strerror(errno),
                      peer_addr.GetHost().c_str())
            if (errno == EPIPE || errno == ECONNRESET) {  // peer disconnected
                if (callback.OnDisconnected) {
                    callback.OnDisconnected(this, DateTime::Now());
                }
                return;
            }
        }
    }

    LOG_DEBUG("Write %zu bytes to fd = %d, remains %zu bytes",
              n,
              channel->GetFd(),
              send_buffer.ReadableBytes())

    // if no data to send, deactivate writing to prevent busy-loop(LT mode)
    if (send_buffer.ReadableBytes() == 0) {
        channel->DeactivateWriting();
    }
}

void TcpConn::OnChannelClose(const DateTime& time) {
    ExchangeState(State::kDisconnected);
    channel->RemoveFromPoller();

    // TcpConn object may be destroyed in an uncertain future, so we release the buffer
    // immediately after peer closed to prevent the garbage data from surviving too long
    send_buffer.Release();
    recv_buffer.Release();

    if (callback.OnDisconnected) {
        callback.OnDisconnected(this, time);
    }
    // when the last shared_ptr of this TcpConn be removed, this TcpConn will be destroyed
    // maybe someone else owns the copy of shared_ptr(such as timer task)
    RemoveFromIOWorker();
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