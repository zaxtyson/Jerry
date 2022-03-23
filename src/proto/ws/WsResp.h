//
// Created by zaxtyson on 2022/3/23.
//

#ifndef JERRY_WSRESP_H
#define JERRY_WSRESP_H

#include <string>
#include <string_view>
#include "WsFrameFormat.h"

namespace jerry::proto::ws {

class WsResp {
  public:
    void SetFrameType(WsFrameType type);
    void AppendData(std::string_view data);
    std::string_view GetPayload() const;

  private:
    WsFrameType type{WsFrameType::kText};
    std::string payload{};
};

}  // namespace jerry::proto::ws


#endif  // JERRY_WSRESP_H
