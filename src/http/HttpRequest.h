//
// Created by zaxtyson on 2021/9/29.
//

#ifndef JERRY_HTTPREQUEST_H
#define JERRY_HTTPREQUEST_H

#include <utils/NonCopyable.h>
#include <unordered_map>
#include <string>

class HttpContext;

/**
 * Http 请求类的封装
 */
class HttpRequest : NonCopyable {
public:
    using Headers = std::unordered_map<std::string, std::string>;
public:
    friend class HttpContext;

public:
    // 请求方法
    enum class Method {
        kGet, kPost, kHead, kOptions, kPut, kDelete, kInvalid
    };
    // 协议版本
    enum class Version {
        kHttpUnknown, kHttp10, kHttp11
    };

public:
    HttpRequest() = default;

    ~HttpRequest() = default;

    /**
     * 获取请求方法
     * @return
     */
    Method getMethod() const { return method_; }

    /**
     * 获取客户端使用的 HTTP 版本
     * @return
     */
    Version getVersion() const { return version_; }

    /**
     * 获取请求路径
     * @return 如: /foo/bar/index.html.back
     */
    const std::string& getRequestURI() const { return path_; }

    /**
     * 获取 URI 不包含文件名的部分
     * @return : /foo/bar
     */
    std::string getServletPath() const;

    /**
     * 获取请求参数
     * @return 如: foo=bar&key=value
     */
    std::string getQueryString() const { return queryString_; };

    /**
     * 获取请求携带的参数值
     * @param key 参数的 key
     * @return value, 如果没找到返回空字符串
     */
    std::string getParameter(const std::string &key) const;

    /**
     * 获取一项 Header
     * @param key 键
     * @return 对应的值或者空字符串
     */
    std::string getHeader(const std::string &key);

    /**
     * 获取全部的 Headers
     * @return
     */
    const Headers& getHeaders() const { return headers_; }

    /**
     * 获取请求体
     * @return
     */
    const std::string& getBody() const { return body_; }

    /**
     * 获取原始请求行
     * @return
     */
    const std::string& getRawRequestLine() const { return rawRequestLine_; }

private:

    void clear();

    void setRawRequestLine(const std::string &line){ rawRequestLine_ = line; };

    void setBody(std::string &&body) { body_ = body; }

    void setMethod(const std::string &method);

    void setRequestURI(const std::string &path) { path_ = path; }

    void setQueryString(const std::string &queryString) { queryString_ = queryString; }

    void setVersion(const std::string &version);

    void addHeader(const std::string &key, const std::string &value) { headers_[key] = value; }


private:
    Version version_{Version::kHttpUnknown};
    Method method_{Method::kInvalid};
    std::string rawRequestLine_;
    std::string path_;  //  /foo/bar/index.html.back
    std::string queryString_; // foo=bar&key=value
    Headers headers_{};
    std::string body_;
};


#endif //JERRY_HTTPREQUEST_H
