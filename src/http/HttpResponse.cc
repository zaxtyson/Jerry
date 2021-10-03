//
// Created by zaxtyson on 2021/9/29.
//

#include "HttpResponse.h"

std::map<HttpResponse::StatusCode, std::string> HttpResponse::httpStatusCodeMap_ = {
        {HttpResponse::StatusCode::kOk,                  "OK"},
        {HttpResponse::StatusCode::kNotFound,            "Not Found"},
        {HttpResponse::StatusCode::kFound,               "Found"},
        {HttpResponse::StatusCode::kPartialContent,      "Partial Content"},
        {HttpResponse::StatusCode::kBadRequest,          "Bad Request"},
        {HttpResponse::StatusCode::kForbidden,           "Forbidden"},
        {HttpResponse::StatusCode::kInternalServerError, "Internal Server Error"},
        {HttpResponse::StatusCode::kMethodNotAllowed,    "Method Not Allowed"},
        {HttpResponse::StatusCode::kMovedPermanently,    "Moved Permanently"},
        {HttpResponse::StatusCode::kTemporaryRedirect,   "Temporary Redirect"}
};

std::string HttpResponse::toString() const {
    std::string ret;
    // 首行
    ret.append("HTTP/1.1 ");
    ret.append(std::to_string(static_cast<int>(statusCode_)));
    ret.append(" ");
    ret.append(httpStatusCodeMap_[statusCode_]);
    ret.append("\r\n");
    // Headers
    ret.append("Server: " SERVER_NAME_AND_VERSION "\r\n");
    ret.append("Connection: keep-alive\r\n"); // 我们不主动 close
    ret.append("Content-Length: ");
    ret.append(std::to_string(writer_.getData().size()));
    ret.append("\r\n");

    if (!containsHeader("Content-Type")) {
        ret.append("Content-Type: text/html;charset=UTF-8\r\n");  // 如果没设置, 默认 html
    }

    for (const auto &item: headers_) {
        ret.append(item.first);
        ret.append(": ");
        ret.append(item.second);
        ret.append("\r\n");
    }

    ret.append("\r\n");
    ret.append(writer_.getData());
    return ret;
}

bool HttpResponse::containsHeader(const std::string &key) const {
    return headers_.find(key) != headers_.end();
}

void HttpResponse::sendRedirect(const std::string &location) {
    setStatus(StatusCode::kTemporaryRedirect); // use 307 redirect
    addHeader("Location", location);
}
