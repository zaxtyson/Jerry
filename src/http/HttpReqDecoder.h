//
// Created by zaxtyson on 2022/3/18.
//

#ifndef JERRY_HTTPRESPDECODER_H
#define JERRY_HTTPRESPDECODER_H

#include "HttpResp.h"
#include <codec/Decoder.h>

namespace jerry::http {

class HttpRespDecoder : public codec::BaseDecoder<HttpResp> {
  public:
    explicit HttpRespDecoder(BaseDecoder::buffer_type& buffer) : BaseDecoder<HttpResp>(buffer) {}

    std::optional<HttpResp> Decode() override { return std::nullopt; }
};

}  // namespace jerry::http

#endif  // JERRY_HTTPRESPDECODER_H
