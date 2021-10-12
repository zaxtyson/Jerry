//
// Created by zaxtyson on 2021/9/29.
//


//    GET /index.html HTTP/1.1
//    Accept: */*
//    Accept-Encoding: gzip, deflate
//    Connection: keep-alive
//    Host: 127.0.0.1
//    User-Agent: HTTPie/2.5.0


#include <algorithm>
#include "HttpContext.h"
#include <thread>
#include <utils/log/Logger.h>

#define CRLF "\r\n"

bool HttpContext::parseRequestLine(const char *begin, const char *end) {
    auto space1 = std::find(begin, end, ' ');
    if (space1 == end) return false;
    request_.setMethod(std::string(begin, space1));
    if (request_.getMethod() == HttpRequest::Method::kInvalid) return false;

    begin = space1 + 1;
    auto space2 = std::find(begin, end, ' ');
    if (space2 == end) return false;

    auto uri = std::string(begin, space2);
    if (uri[0] != '/') return false; // 路径不以 / 开头, 当然, 这里可以做更精细的检查

    // fragment 不要
    auto fragment = uri.find('#');
    if (fragment != std::string::npos) {
        uri = uri.substr(0, fragment);
    }

    // 如果有 query 参数
    auto query = uri.find('?');
    if (query != std::string::npos) {
        request_.setRequestURI(std::string(uri, 0, query));
        request_.setQueryString(std::string(uri, query + 1));
    } else {
        request_.setRequestURI(uri);
    }

    // 直接取 HTTP/1.x 最后一个字符
    auto version = std::string(end - 1, end);
    if (version != "0" && version != "1") return false;  // 有问题
    request_.setVersion(version);
    return true;
}

bool HttpContext::parseHeaderLine(const char *begin, const char *end) {
    auto delimiter = std::find(begin, end, ':');
    if (delimiter == end) return false;
    request_.addHeader(std::string(begin, delimiter), std::string(delimiter + 2, end));
    return true;
}

bool HttpContext::parseRequest(MsgBuffer &buffer) {
    // 解析过程异常才返回失败, 数据不完整可以等到下一次获取数据接着解析
    bool isParseSuccess = true;
    LOG_DEBUG("Parse request, data length: %lu", buffer.readableBytes());

    while (true) {
        if (state_ == ParserState::kExpectRequestLine) {
            auto crlf = buffer.search(CRLF);
            if (crlf == buffer.readEnd()) break; // 数据暂不完整
            // 读到一行完整数据
            if (!parseRequestLine(buffer.readBegin(), crlf)) {
                isParseSuccess = false; // 解析请求行失败
                LOG_DEBUG("Parse request line failed: %s", buffer.popAll().c_str());
                buffer.dropAll(); // 清空无效数据, 虽然日志pop了一次,关掉日志也得清空数据
                break;
            }
            buffer.dropUntil(crlf + 2); // 移除数据, 顺带把 \r\n 也移除
            state_ = ParserState::kExpectHeaders; // 进入下一个状态
        } else if (state_ == ParserState::kExpectHeaders) {
            auto crlf = buffer.search(CRLF);
            if (crlf == buffer.readEnd()) break;

            // 头部已经解析完成
            if (buffer.startsWith(CRLF)) {
                buffer.drop(2);
                state_ = ParserState::kExpectBody;
                continue;  // 进入下一个状态
            }

            // 解析一行头部
            if (!parseHeaderLine(buffer.readBegin(), crlf)) {
                isParseSuccess = false;
                LOG_DEBUG("Parse headers line failed: %s", buffer.popAll().c_str());
                buffer.dropAll(); // 清空无效数据
                break;
            }
            buffer.dropUntil(crlf + 2);
        } else if (state_ == ParserState::kExpectBody) {
            // 剩下的作为请求数据
            request_.setBody(std::move(buffer.popAll()));
            state_ = ParserState::kParseFinished;
            break;
        }
    }

    return isParseSuccess;
}

void HttpContext::reset() {
    request_.clear();
    state_ = ParserState::kExpectRequestLine;
}
