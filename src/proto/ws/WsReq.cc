//
// Created by zaxtyson on 2022/3/23.
//

#include "WsReq.h"

namespace jerry::proto::ws {

std::string_view WsReq::GetPayload() const {
    return payload;
}

size_t WsReq::GetPayloadLength() const {
    return payload.size();
}

WsFrameType WsReq::GetFrameType() const {
    return type;
}

std::string_view WsReq::GetRequestUri() const {
    return request_uri;
}

std::string_view WsReq::GetHost() const {
    return host;
}

}  // namespace jerry::proto::ws
