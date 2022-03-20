//
// Created by zaxtyson on 2022/3/18.
//

#ifndef JERRY_HTTPREQ_H
#define JERRY_HTTPREQ_H

#include <cstring>
#include <map>
#include <string>
#include <string_view>

namespace jerry::http {

class HttpReqDecoder;

struct CaseInsensitiveComparator {
    bool operator()(const std::string& lhs, const std::string& rhs) const noexcept {
        return ::strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};

class HttpReq {
  public:
    using Headers = std::map<std::string, std::string, CaseInsensitiveComparator>;
    using Body = std::string;

  public:
    enum class Method : uint8_t { OPTIONS, GET, HEAD, POST, PUT, DELETE, TRACE, CONNECT };
    enum class HttpVersion : uint8_t { Http10, Http11 /** Http20**/ };

  public:
    std::string_view GetBody() const;
    std::string_view GetRequestUri() const;
    bool IsBodyTruncated() const;
    std::optional<std::string_view> GetHeader(std::string_view name) const;
    HttpVersion GetVersion() const;
    Method GetMethod() const;
    std::string_view GetMethodString() const;
    std::string_view GetVersionString() const;
    std::string ToString() const;

  private:
    friend class HttpReqDecoder;
    Body body{};
    std::string request_uri{};
    Headers headers{};
    Method method;
    HttpVersion version;
    bool is_body_truncated{false};

  private:
    static const std::map<Method, std::string> method_map;
    static const std::map<HttpVersion, std::string> version_map;
};

}  // namespace jerry::http
#endif  // JERRY_HTTPREQ_H
