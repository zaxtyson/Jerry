//
// Created by zaxtyson on 2021/9/29.
//

#include <algorithm>
#include "HttpRequest.h"

std::string HttpRequest::getHeader(const std::string &key) {
    if (headers_.find(key) != headers_.end()) {
        return headers_[key];
    }
    return "";
}

void HttpRequest::setMethod(const std::string &method) {
    if (method == "GET") {
        method_ = Method::kGet;
    } else if (method == "POST") {
        method_ = Method::kPost;
    } else if (method == "PUT") {
        method_ = Method::kPut;
    } else if (method == "DELETE") {
        method_ = Method::kDelete;
    } else if (method == "HEAD") {
        method_ = Method::kHead;
    } else if (method == "OPTIONS") {
        method_ = Method::kOptions;
    } else {
        method_ = Method::kInvalid;
    }
}

void HttpRequest::setVersion(const std::string &version) {
    if (version == "1") {
        version_ = Version::kHttp11;
    } else if (version == "0") {
        version_ = Version::kHttp10;
    }
}

void HttpRequest::clear() {
    method_ = Method::kInvalid;
    version_ = Version::kHttpUnknown;
    path_.clear();
    body_.clear();
    headers_.clear();
}

std::string HttpRequest::getServletPath() const {
    // /foo/bar/index.html.back -> /foo/bar
    // /foo/bar -> /foo
    // foo/bar/ -> /foo/bar
    // /foo -> /
    // / -> /
    auto pos = path_.find_last_of('/');
    if (pos == 0) return "/";
    return path_.substr(0, pos);
}

std::string HttpRequest::getParameter(const std::string &key) const {
    // foo=bar&blank=&key=value
    // foo=bar
    // blank=
    auto startOfKey = queryString_.find(key + "=");
    if (startOfKey == std::string::npos) return "";
    auto startOfValue = startOfKey + key.length() + 1;
    auto endOfValue = queryString_.find('&', startOfValue);
    if (endOfValue == std::string::npos) endOfValue = queryString_.length();
    return queryString_.substr(startOfValue, endOfValue - startOfValue);
}
