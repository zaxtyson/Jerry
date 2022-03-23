//
// Created by zaxtyson on 2022/3/18.
//

#include "HttpReqDecoder.h"
#include <algorithm>
#include <cassert>

namespace jerry::proto::http {

constexpr auto CRLF = "\r\n";

const std::map<std::string, HttpReq::Method> HttpReqDecoder::method_map = {
    {"GET", HttpReq::Method::GET},
    {"POST", HttpReq::Method::POST},
    {"HEAD", HttpReq::Method::HEAD},
    {"HEAD", HttpReq::Method::HEAD},
    {"HEAD", HttpReq::Method::HEAD},
    {"DELETE", HttpReq::Method::DELETE},
    {"OPTIONS", HttpReq::Method::OPTIONS},
    {"TRACE", HttpReq::Method::TRACE},
};

const std::map<std::string, HttpReq::HttpVersion> HttpReqDecoder::version_map = {
    {"HTTP/1.0", HttpReq::HttpVersion::Http10},
    {"HTTP/1.1", HttpReq::HttpVersion::Http11}};

HttpReqDecoder::HttpReqDecoder(net::BaseBuffer& buffer) : buffer{buffer} {}

std::optional<HttpReq> HttpReqDecoder::Decode() {
    while (true) {
        switch (state) {
            case State::kExpectRequestLine:
                // ensure `req` is clear
                assert(req.headers.empty());
                assert(req.body.empty());

                if (!ParseRequestLine()) {
                    return std::nullopt;
                }
                break;
            case State::kExpectHeaders:
                if (!ParseHeaders()) {
                    return std::nullopt;
                }
                break;
            case State::kExpectBody:
                ParseBody();  // always true
                break;
            case State::kParseFinished:
                // let's wait for next request
                state = State::kExpectRequestLine;
                return std::move(req);  // move data out
        }
    }
}

void HttpReqDecoder::SetBodyMaxBytes(size_t n) {
    this->body_max_bytes = n;
}

bool HttpReqDecoder::ParseRequestLine() {
    // Request-Line = Method SP Request-URI SP HTTP-Version CRLF

    // parse http method
    auto sp1 = std::find(buffer.BeginOfReadable(), buffer.EndOfReadable(), ' ');
    if (sp1 == buffer.EndOfReadable()) {
        // the buffer ReadableBytes >= 7, but we can't find ' ', must something wrong here!
        // the max length of method strings is 7 ("OPTION ")
        if (buffer.ReadableBytes() >= 7) {
            buffer.DropAllBytes();
        }
        return false;
    }
    std::string method_str{buffer.BeginOfReadable(), sp1};
    auto method_iter = method_map.find(method_str);
    if (method_iter == std::end(method_map)) {
        // there something wrong! the data is invalid, drop it!
        buffer.DropAllBytes();
        return false;
    }
    req.method = method_iter->second;

    // parse http request uri
    auto sp2 = std::find(sp1 + 1, buffer.EndOfReadable(), ' ');
    if (sp2 == buffer.EndOfReadable()) {
        return false;
    }
    req.request_uri = {sp1 + 1, sp2};

    // parse http version
    auto crlf = std::search(sp2 + 1, buffer.EndOfReadable(), CRLF, CRLF + 2);
    if (crlf == buffer.EndOfReadable()) {
        return false;
    }
    std::string version_str{sp2 + 1, crlf};
    req.version = version_map.at(version_str);

    // parse finish, drop parsed data and change state
    buffer.DropBytesUntil(crlf + 2);
    state = State::kExpectHeaders;
    return true;
}

bool HttpReqDecoder::ParseHeaders() {
    // name1: value1\r\n
    // name2: value2\r\n
    // ...
    // \r\n
    while (true) {
        // reach the End of headers
        if (*buffer.BeginOfReadable() == '\r' && *next(buffer.BeginOfReadable()) == '\n') {
            break;
        }

        // parse one line
        auto colon = std::find(buffer.BeginOfReadable(), buffer.EndOfReadable(), ':');
        if (colon == buffer.EndOfReadable()) {
            return false;
        }
        auto crlf = std::search(colon + 1, buffer.EndOfReadable(), CRLF, CRLF + 2);
        if (crlf == buffer.EndOfReadable()) {
            return false;
        }
        std::string name{buffer.BeginOfReadable(), colon};
        // whitespace before the value is ignored
        auto value_pos = (*(colon + 1) == ' ') ? colon + 2 : colon + 1;
        std::string value{value_pos, crlf};
        req.headers.emplace(std::move(name), std::move(value));
        buffer.DropBytesUntil(crlf + 2);
    }

    // parse headers finished
    buffer.DropBytes(2);  // drop last CRLF
    state = State::kExpectBody;
    return true;
}

void HttpReqDecoder::ParseBody() {
    auto max_size = std::min(buffer.ReadableBytes(), body_max_bytes);
    if (max_size > body_max_bytes) {
        req.is_body_truncated = true;  // mark it as true
    }
    req.body = {buffer.BeginOfReadable(), buffer.BeginOfReadable() + max_size};
    state = State::kParseFinished;
    buffer.DropAllBytes();
}

}  // namespace jerry::proto::http