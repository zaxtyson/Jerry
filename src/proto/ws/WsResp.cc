//
// Created by zaxtyson on 2022/3/23.
//

#include "WsResp.h"

namespace jerry::proto::ws {

void WsResp::SetFrameType(WsFrameType type) {
    this->type = type;
}

void WsResp::AppendData(std::string_view data) {
    this->payload.append(data);
}

std::string_view WsResp::GetPayload() const {
    return payload;
}

}  // namespace jerry::proto::ws