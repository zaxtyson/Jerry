//
// Created by zaxtyson on 2022/3/23.
//

#ifndef JERRY_WSREQ_H
#define JERRY_WSREQ_H

#include <string>
#include <string_view>
#include "WsFrameFormat.h"

namespace jerry::proto::ws {

class WebsocketCodec;

class WsReq {
  public:
    std::string_view GetPayload() const;
    std::string_view GetHost() const;
    std::string_view GetRequestUri() const;
    size_t GetPayloadLength() const;
    WsFrameType GetFrameType() const;

  private:
    friend class WebsocketCodec;

  private:
    WsFrameType type;
    std::string payload{};
    std::string host{};
    std::string request_uri{};
};

}  // namespace jerry::proto::ws


#endif  // JERRY_WSREQ_H
