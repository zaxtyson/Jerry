//
// Created by zaxtyson on 2022/3/12.
//

#ifndef JERRY_TCPCONN_H
#define JERRY_TCPCONN_H

#include <any>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string_view>
#include "BaseBuffer.h"
#include "InetAddress.h"
#include "utils/DateTime.h"
#include "utils/NonCopyable.h"

namespace jerry::net {

class Channel;
class IOWorker;
class TcpConn;
class TimerQueue;

using LockGuard = std::lock_guard<std::mutex>;

enum class TcpState : char { kConnected, kDisconnected, kShutdown };

struct TcpCallback {
    using Type = std::function<void(TcpConn*, const DateTime&)>;

    Type OnConnected = nullptr;
    Type OnDisconnected = nullptr;
    Type OnDataReceived = nullptr;
};


class TcpContext {
  public:
    std::optional<std::any> Get(std::string_view key) const;
    void Set(std::string_view key, const std::any& value);

  private:
    mutable std::mutex mtx{};
    std::map<std::string, std::any> context{};
};


class TcpConn : NonCopyable, public std::enable_shared_from_this<TcpConn> {
  public:
    TcpConn(int fd, const InetAddress& local_addr, const InetAddress& peer_addr);
    ~TcpConn();

  public:
    /**
     * Check if the current connection is connected
     * @return true if the connection is connected
     */
    bool IsConnected() const;

    /**
     * Get the channel bounded with this connection
     * @return the pointer of channel
     */
    Channel* GetChannel() const;

    /**
     * Get the IOWorker(EventLoop) that the connection belongs to
     * @return the pointer of IOWorker
     */
    IOWorker* GetIOWorker() const;

    /**
     * Get the shared_ptr of this TcpConn
     * We MUST use a shared_ptr in the ThreadPool or Timer instead of the bare pointer,
     * because when the asynchronous task is executed, the tcp connection may have been
     * broken and the TcpConn object has been destroyed, so we need to ensure that the
     * TcpConn object lives longer than the asynchronous task.
     * NEVER create the shared_ptr of a TcpConn with its bare pointer manually
     * @return the shared_ptr of this TcpConn
     */
    std::shared_ptr<TcpConn> GetSharedPtr();

    /**
     * Get the TimerQueue in IOWorker
     * @return the pointer of TimerQueue
     */
    TimerQueue* GetTimerQueue() const;

    /**
     * Get the buffer which stores the data we received
     * @return the reference of recv buffer
     */
    BaseBuffer& GetRecvBuffer();

    /**
     * Get the buffer which stores the data to be send
     * @return the reference of send buffer
     */
    BaseBuffer& GetSendBuffer();

    /**
     * Get the listen address of server
     * @return the listen address of server
     */
    const InetAddress& GetLocalAddress() const;

    /**
     * Get the address of client
     * @return the address of client
     */
    const InetAddress& GetPeerAddress() const;

    /**
     * Get the codec bind with this TcpConn
     * @return the codec object
     */
    template <typename Ret>
    Ret GetDecoder() const {
        return std::any_cast<Ret>(codec);
    }

    /**
     * Get the context object set by user
     * @param key the key of context
     * @return the object of context
     */
    template <typename Ret>
    std::optional<Ret> GetContext(std::string_view key) {
        auto ret = context.Get(key);
        if (!ret.has_value()) {
            return std::nullopt;
        }
        return std::any_cast<Ret>(ret.value());
    }

  public:
    /**
     * Bind the IoWorker which manages this connection
     * @param worker the pointer of IoWorker
     */
    void SetIOWorker(IOWorker* worker);

    /**
     * Set a context for this connection
     * NOTE: the value of context will be overwritten if the key is existed.
     * if the old value is a pointer, be sure the object will be released correctly,
     * you'd better using the share_ptr as context value rather than the bare pointer
     *
     * @param key the key of context
     * @param value the value of context
     */
    void SetContext(std::string_view key, const std::any& value);

    /**
     * Set callbacks for this connection
     * @param callback the callback functions
     */
    void SetCallback(const TcpCallback& callback);

    /**
     * Set a codec for this TcpConn to decode the byte stream
     * @param codec the codec(it's better to use a shared_ptr)
     */
    void SetStreamCodec(const std::any& codec);

  public:
    /**
     * Write the data to send_buffer, and it will be sent automatically
     * @param data the data to send
     */
    void Send(std::string_view data);
    void Send(const BaseBuffer::value_type* data, size_t len);
    void Send(BaseBuffer::const_iterator begin, size_t len);
    void Send(BaseBuffer::const_iterator begin, BaseBuffer::const_iterator end);
    // void SendFile();

    /**
     * Shutdown this connection
     * Just send a `FIN` to client and mark this connection as `TcpState::kShutdown`.
     * You can no longer send any data, but you can continue to read from client.
     * Connection will completely closed when the client disconnected.
     */
    void Shutdown();

    /**
     * Close the connection
     */
    void ForceClose();

  private:
    TcpState ExchangeState(TcpState state);
    void RemoveFromIOWorker();

  private:
    Channel* channel{};
    IOWorker* worker{};
    const InetAddress local_addr;
    const InetAddress peer_addr;
    BaseBuffer send_buffer{1024};
    BaseBuffer recv_buffer{1024};
    TcpContext context{};
    std::any codec;
    std::atomic<TcpState> state{TcpState::kConnected};
    TcpCallback callback{};
};


using TcpConnPtr = std::shared_ptr<TcpConn>;


class TcpConnSet {
  public:
    bool Empty() const;
    size_t Size() const;
    void Add(TcpConn* conn);
    void Add(const TcpConnPtr& conn);
    void Remove(const TcpConnPtr& conn);
    void LockForEach(std::function<void(const TcpConnPtr&)>&& func) const;

  private:
    std::set<TcpConnPtr> conn_set{};
    mutable std::mutex mtx{};
};

}  // namespace jerry::net


#endif  // JERRY_TCPCONN_H
