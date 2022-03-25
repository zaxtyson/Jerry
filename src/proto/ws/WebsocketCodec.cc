//
// Created by zaxtyson on 2022/3/22.
//

#include "WebsocketCodec.h"
#include <endian.h>
#include "WsResp.h"
#include "crypto/base64.h"
#include "crypto/sha1.h"
#include "logger/Logger.h"
#include "proto/http/HttpReqDecoder.h"
#include "proto/http/HttpResp.h"

namespace jerry::proto::ws {

constexpr const char* RFC6544_MAGIC_KEY = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

WebsocketCodec::WebsocketCodec(net::TcpConn* conn) : conn{conn} {}

std::optional<WsReq> WebsocketCodec::Decode() {
    while (true) {
        switch (state) {
            case State::kExceptHandshake:
                HandShake();
                return std::nullopt;  // no return no matter success or failed
            case State::kExceptFrameHead:
                if (!DecodeFrameHead()) {
                    return std::nullopt;
                }  // if success, go on
                break;
            case State::kExceptPayload:
                if (!DecodePayload()) {
                    return std::nullopt;  // req is not complete
                }
                break;
            case State::kParseFinished:
                state = State::kExceptFrameHead;  // wait for next frame
                return std::move(req);
        }
    }
}

void WebsocketCodec::HandShake() {
    http::HttpReqDecoder http_decoder(conn->GetRecvBuffer());
    auto http_req = http_decoder.Decode();
    if (!http_req.has_value()) {
        return;  // http request not incomplete
    }

    http::HttpResp resp;

    // https://datatracker.ietf.org/doc/html/rfc6455#section-3
    // The method of the request MUST be GET, and the HTTP version MUST be at least 1.1.
    if (http_req->GetMethod() != http::HttpReq::Method::GET ||
        http_req->GetVersion() < http::HttpReq::HttpVersion::Http11 ||
        !http_req->GetHeader("Host").has_value() || !http_req->GetHeader("Upgrade").has_value() ||
        !http_req->GetHeader("Connection").has_value() ||
        !http_req->GetHeader("Sec-WebSocket-Version").has_value() ||
        !http_req->GetHeader("Sec-WebSocket-Key").has_value()) {
        resp.SetStatus(400);
        conn->Send(resp.ToString());
        conn->Shutdown();
        return;
    }

    // request is valid, send response to client
    auto sec_key = http_req->GetHeader("Sec-WebSocket-Key");
    auto accept_key = GetSecWebSocketAccept(sec_key.value());

    resp.SetStatus(101);
    resp.AddHeader("Upgrade", "WebSocket");
    resp.AddHeader("Connection", "Upgrade");
    resp.AddHeader("Sec-WebSocket-Accept", std::move(accept_key));
    conn->Send(resp.ToString());
    // LOG_DEBUG("%s", http_req->ToString().data())
    // LOG_DEBUG("%s", resp.ToString().data())

    // backup the host and request_uri info
    host = http_req->GetHeader("Host").value();
    request_uri = http_req->GetRequestUri();

    state = State::kExceptFrameHead;
}

std::string WebsocketCodec::GetSecWebSocketAccept(std::string_view sec_key) {
    std::string accept_key{sec_key};
    accept_key += RFC6544_MAGIC_KEY;
    uint8_t digest[20]{};  // 160 bit sha1 digest
    class SHA1 sha;
    sha.Input(accept_key.data(), accept_key.size());
    sha.Result((uint32_t*)digest);
    // little endian to big endian
    for (auto i = 0; i < 20; i += 4) {
        std::swap(digest[i], digest[i + 3]);
        std::swap(digest[i + 1], digest[i + 2]);
    }
    return base64_encode(digest, 20);
}

bool WebsocketCodec::DecodeFrameHead() {
    auto& recv_buffer = conn->GetRecvBuffer();
    uint8_t* stream = reinterpret_cast<uint8_t*>(recv_buffer.BeginOfReadable().base());

    auto* head = reinterpret_cast<ws_base_head*>(stream);
    LOG_DEBUG("FIN=%d, RSV1=%d, RSV2=%d, RSV3=%d, PAYLOAD_LEN=%d, OPCODE=0x%x, MASKED=%d",
              head->fin,
              head->rsv1,
              head->rsv2,
              head->rsv3,
              head->payload_length,
              head->opcode,
              head->mask)

    // All frames sent from client to server have this bit set to 1.
    if (head->mask == 0 || head->opcode == static_cast<uint8_t>(WsFrameType::kClose)) {
        recv_buffer.DropAllBytes();
        LOG_DEBUG("Close the websocket: %s", conn->GetPeerAddress().GetHost().data())
        // send close frame
        auto close_frame = EncodeFrameHead("", WsFrameType::kClose);
        conn->Send(close_frame);
        conn->Shutdown();
        return false;
    }

    // get the payload length and mask_key
    uint8_t* data = stream + 2;

    if (head->payload_length < 126) {
        payload_remains = head->payload_length;
        mask_key = head->mask_key;
        data += 4;  // mask_key is 4 bytes
    } else if (head->payload_length == 126) {
        auto* head126 = reinterpret_cast<ws_ext_head_126*>(data);
        payload_remains = be16toh(head126->payload_length);
        mask_key = head126->mask_key;
        data += 6;  // 2 bytes payload len + 4 bytes mask_key
    } else if (head->payload_length == 127) {
        auto* head127 = reinterpret_cast<ws_ext_head_127*>(data);
        payload_remains = be64toh(head127->payload_length);
        mask_key = head127->mask_key;
        data += 12;  // 8 bytes payload len + 4 bytes mask_key
    }

    // drop data before payload
    recv_buffer.DropBytes(data - stream);

    // set request info
    req.type = static_cast<WsFrameType>(head->opcode);
    req.host = host;
    req.request_uri = request_uri;

    state = State::kExceptPayload;
    return true;
}


bool WebsocketCodec::DecodePayload() {
    auto& recv_buffer = conn->GetRecvBuffer();
    LOG_DEBUG("Payload remains %zu, recv_buffer: %zu, mask_key=0x%x",
              payload_remains,
              recv_buffer.ReadableBytes(),
              mask_key)

    uint64_t size = std::min(recv_buffer.ReadableBytes(), payload_remains);
    payload_remains -= size;

    // https://datatracker.ietf.org/doc/html/rfc6455#section-5.3
    uint8_t* data = reinterpret_cast<uint8_t*>(recv_buffer.BeginOfReadable().base());
    for (uint64_t i = 0; i < size; i++) {
        // `i` must be the index of this byte in the entire payload
        // this method may be call more than one times(when payload_len=127)
        // so, we should remember the index we parsed last time
        data[i] ^= reinterpret_cast<uint8_t*>(&mask_key)[(i + last_read_idx) % 4];
    }

    last_read_idx += size;
    req.payload.append(recv_buffer.BeginOfReadable().base(), size);
    recv_buffer.DropBytes(size);

    if (payload_remains == 0) {
        last_read_idx = 0;
        state = State::kParseFinished;
        return true;
    }

    // the frame is not complete
    return false;
}

std::string WebsocketCodec::EncodeFrameHead(std::string_view data, WsFrameType type) {
    // [ 2 bytes base head | 0/2/8 bytes length | data ]
    uint8_t encode_head[11]{};

    ws_base_head base_head{};
    base_head.fin = 1;   // this is the last frame
    base_head.mask = 0;  // client -> server must set to 1
    base_head.opcode = static_cast<uint8_t>(type);

    if (data.size() < 126) {
        base_head.payload_length = data.size();
    } else if (data.size() < 65535) {
        base_head.payload_length = 126;
        uint16_t length = htobe16(data.size());  // 2 bytes length
        std::memcpy(encode_head + 2, &length, sizeof(length));
    } else {
        base_head.payload_length = 127;
        uint64_t length = htole64(data.size());  // 8 bytes length
        std::memcpy(encode_head + 2, &length, sizeof(length));
    }
    std::memcpy(encode_head, &base_head, 2);
    return reinterpret_cast<const char*>(encode_head);
}

}  // namespace jerry::proto::ws