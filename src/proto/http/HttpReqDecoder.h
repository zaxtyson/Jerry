//
// Created by zaxtyson on 2022/3/18.
//

#ifndef JERRY_HTTPREQDECODER_H
#define JERRY_HTTPREQDECODER_H

#include <map>
#include "HttpReq.h"
#include "net/BaseBuffer.h"

namespace jerry::proto::http {

class HttpReqDecoder {
  public:
    explicit HttpReqDecoder(net::BaseBuffer& buffer);
    ~HttpReqDecoder() = default;

    std::optional<HttpReq> Decode();
    void SetBodyMaxBytes(size_t n);

  private:
    bool ParseRequestLine();
    bool ParseHeaders();
    void ParseBody();


  private:
    enum class State { kExpectRequestLine, kExpectHeaders, kExpectBody, kParseFinished };

  private:
    HttpReq req{};
    size_t body_max_bytes{2 * 1024 * 1024};  // default 2MB
    State state{State::kExpectRequestLine};
    net::BaseBuffer& buffer;

  private:
    static const std::map<std::string, HttpReq::Method> method_map;
    static const std::map<std::string, HttpReq::HttpVersion> version_map;
};

}  // namespace jerry::proto::http

#endif  // JERRY_HTTPREQDECODER_H
