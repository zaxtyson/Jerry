//
// Created by zaxtyson on 2022/3/18.
//

#include "HttpReq.h"

namespace jerry::http {

const std::map<HttpReq::Method, std::string> HttpReq::method_map = {
    {HttpReq::Method::GET, "GET"},
    {HttpReq::Method::POST, "POST"},
    {HttpReq::Method::HEAD, "HEAD"},
    {HttpReq::Method::HEAD, "HEAD"},
    {HttpReq::Method::HEAD, "HEAD"},
    {HttpReq::Method::DELETE, "DELETE"},
    {HttpReq::Method::OPTIONS, "OPTIONS"},
    {HttpReq::Method::TRACE, "TRACE"}};

const std::map<HttpReq::HttpVersion, std::string> HttpReq::version_map = {
    {HttpReq::HttpVersion::Http10, "HTTP/1.0"},
    {HttpReq::HttpVersion::Http11, "HTTP/1.1"}};

std::string_view HttpReq::GetBody() const {
    return body;
}

std::string_view HttpReq::GetRequestUri() const {
    return request_uri;
}

std::optional<std::string_view> HttpReq::GetHeader(std::string_view name) const {
    auto target = headers.find(name.data());
    if (target == std::end(headers)) {
        return std::nullopt;
    }
    return std::string_view(target->second);
}

HttpReq::HttpVersion HttpReq::GetVersion() const {
    return version;
}

HttpReq::Method HttpReq::GetMethod() const {
    return method;
}

std::string_view HttpReq::GetMethodString() const {
    return method_map.at(method);
}

std::string_view HttpReq::GetVersionString() const {
    return version_map.at(version);
}

bool HttpReq::IsBodyTruncated() const {
    return is_body_truncated;
}

std::string HttpReq::ToString() const {
    std::string req_str;
    // request line
    req_str.append(GetMethodString());
    req_str.append(" ");
    req_str.append(GetRequestUri());
    req_str.append(" ");
    req_str.append(GetVersionString());
    req_str.append("\n");

    // headers
    for (auto& [name, value] : headers) {
        req_str.append(name);
        req_str.append(": ");
        req_str.append(value);
        req_str.append("\n");
    }

    // body
    req_str.append("\n");
    req_str.append(GetBody());
    return req_str;
}

}  // namespace jerry::http