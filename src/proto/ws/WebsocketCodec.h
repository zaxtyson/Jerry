//
// Created by zaxtyson on 2022/3/22.
//

#ifndef JERRY_WEBSOCKETDECODER_H
#define JERRY_WEBSOCKETDECODER_H

#include "WsFrameFormat.h"
#include "WsReq.h"
#include "net/TcpConn.h"

namespace jerry::proto::ws {

class WebsocketCodec {
  public:
    explicit WebsocketCodec(net::TcpConn* conn);
    ~WebsocketCodec() = default;

    /**
     * Decode the stream in recv_buffer(auto handshake)
     * it will return `std::nullopt` when do handshake or decode the frame failed
     *
     * @return a string_view reference the decoded data in recv_buffer to avoid of copy
     * `conn->recv_buffer->DropAllBytes()` should be called to release the data
     * construct a std::string if handle the data in async task or timer
     */
    std::optional<WsReq> Decode();

    /**
     * Encode the websocket frame head of data to send
     * @param data the data to send
     * @param type the frame type
     * @return encoded bytes
     */
    std::string EncodeFrameHead(std::string_view data, WsFrameType type = WsFrameType::kText);

  private:
    void HandShake();
    bool DecodeFrameHead();
    bool DecodePayload();
    std::string GetSecWebSocketAccept(std::string_view sec_key);

  private:
    enum class State { kExceptHandshake, kExceptFrameHead, kExceptPayload, kParseFinished };

  private:
    WsReq req{};
    std::string request_uri{};  // backup when handshake
    std::string host{};
    uint32_t mask_key{};
    uint64_t payload_remains{};  // remains payload to read
    uint64_t last_read_idx{};
    State state{State::kExceptHandshake};
    net::TcpConn* conn;
};

}  // namespace jerry::proto::ws


#endif  // JERRY_WEBSOCKETDECODER_H
