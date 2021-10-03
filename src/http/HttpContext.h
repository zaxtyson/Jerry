//
// Created by zaxtyson on 2021/9/29.
//

#ifndef JERRY_HTTPCONTEXT_H
#define JERRY_HTTPCONTEXT_H

#include <utils/NonCopyable.h>
#include <http/HttpRequest.h>
#include <net/MsgBuffer.h>

class HttpContext : NonCopyable {
public:
    // 状态机的状态
    enum class ParserState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kParseFinished
    };

public :
    HttpContext() = default;

    ~HttpContext() = default;

    /**
     * 解析请求
     * @param buffer
     * @return 解析失败返回 false, 解析成功(即便数据不全)返回 true
     */
    bool parseRequest(MsgBuffer &buffer);

    /**
     * 获取解析出来的请求对象
     * @return
     */
    HttpRequest &getRequest() { return request_; }

    /**
     * 重置 Http 状态机
     */
    void reset();

    /**
     * 是否完整地解析了一个请求
     * @return
     */
    bool isParseFinished() { return state_ == ParserState::kParseFinished; }


private:
    bool parseRequestLine(const char *begin, const char *end);

    bool parseHeaderLine(const char *begin, const char *end);

private:
    HttpRequest request_;
    ParserState state_{ParserState::kExpectRequestLine};
};


#endif //JERRY_HTTPCONTEXT_H
