//
// Created by zaxtyson on 2021/9/29.
//

#ifndef JERRY_HTTPRESPONSE_H
#define JERRY_HTTPRESPONSE_H

#ifndef SERVER_NAME_AND_VERSION
#define SERVER_NAME_AND_VERSION "JerryServer/0.1.0"
#endif

#include <utils/NonCopyable.h>
#include <http/PrintWriter.h>
#include <string>
#include <map>

class HttpServer;

class HttpResponse : NonCopyable {
public:
    using Headers = std::map<std::string, std::string>;
public:
    friend class HttpServer;

public :
    // Http 状态码
    enum class StatusCode : int {
        kOk = 200,
        kPartialContent = 206,
        kMovedPermanently = 301,
        kFound = 302,
        kTemporaryRedirect = 307,
        kBadRequest = 400,
        kForbidden = 403,
        kNotFound = 404,
        kMethodNotAllowed = 405,
        kInternalServerError = 500
    };
public:
    HttpResponse() = default;

    ~HttpResponse() = default;

    /**
     * 设置响应状态码
     * @param status
     */
    void setStatus(StatusCode status) { statusCode_ = status; };

    /**
     * 添加一项 Header
     * @param key
     * @param value
     */
    void addHeader(const std::string &key, const std::string &value) { headers_[key] = value; }

    /**
     * 设置响应头的 Content-Type 字段
     * @param contentType
     */
    void setContentType(const std::string &contentType) { addHeader("Content-Type", contentType); }

    /**
     * 用于向客户端写数据
     * @return
     */
    PrintWriter *getWriter() { return &writer_; };

    /**
     * 响应头是否包含给定的字段
     * @param key
     * @return
     */
    bool containsHeader(const std::string &key) const;

    /**
     * 发送重定向响应
     * @param location 重定向的 URL
     */
    void sendRedirect(const std::string &location);

private:
    std::string toString() const;

private:
    StatusCode statusCode_{StatusCode::kOk};
    Headers headers_;
    PrintWriter writer_;
    static std::map<StatusCode, std::string> httpStatusCodeMap_;
};


#endif //JERRY_HTTPRESPONSE_H
