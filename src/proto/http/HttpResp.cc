//
// Created by zaxtyson on 2022/3/18.
//

#include "HttpResp.h"
#include "utils/DateTime.h"

namespace jerry::proto::http {

constexpr const char* CRLF = "\r\n";

// https://datatracker.ietf.org/doc/html/rfc2616#section-6.1.1
const std::map<HttpResp::Status, std::string> HttpResp::status_map = {
    {100, "Continue"},
    {101, "Switching Protocols"},
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritative Information"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {300, "Multiple Choices"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {305, "Use Proxy"},
    {307, "Temporary Redirect"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Time-out"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Precondition Failed"},
    {413, "Request Entity Too Large"},
    {414, "Request-URI Too Large"},
    {415, "Unsupported Media Type"},
    {416, "Requested range not satisfiable"},
    {417, "Expectation Failed"},
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Time-out"},
    {505, "HTTP Version not supported"}};

void HttpResp::AddHeader(std::string_view name, std::string_view value, bool multiple) {
    /**
     * https://datatracker.ietf.org/doc/html/rfc2616#section-4.2
     *
     * Multiple message-header fields with the same field-name MAY be
     * present in a message if and only if the entire field-value for that
     * header field is defined as a comma-separated list [i.e., #(values)].
     * It MUST be possible to combine the multiple header fields into one
     * "field-name: field-value" pair, without changing the semantics of the
     * message, by appending each subsequent field-value to the first, each
     * separated by a comma. The order in which header fields with the same
     * field-name are received is therefore significant to the
     * interpretation of the combined field value, and thus a proxy MUST NOT
     * change the order of these field values when a message is forwarded.
     */

    if (multiple) {
        headers.emplace(name, value);
        return;
    }

    auto target = headers.find(name.data());
    if (target == std::end(headers)) {
        headers.emplace(name, value);
    } else {
        target->second = value;  // overwrite the old value
    }
}

void HttpResp::SetContentType(std::string_view content_type) {
    AddHeader("Content-Type", content_type);
}

void HttpResp::SetServer(std::string_view server_name) {
    AddHeader("Server", server_name);
}

void HttpResp::SetRedirect(std::string_view location, Status status_code) {
    this->status_code = status_code;
    AddHeader("Location", location);
}

void HttpResp::AddEtag(std::string_view etag) {
    AddHeader("Etag", etag, true);
}

void HttpResp::SetKeepAlive() {
    AddHeader("Connection", "keep-Alive");
}

void HttpResp::SetStatus(HttpResp::Status status_code) {
    this->status_code = status_code;
}

void HttpResp::SetBody(std::string_view body) {
    this->body = body;
}

void HttpResp::AppendBody(std::string_view body) {
    this->body.append(body);
}

std::optional<std::string_view> HttpResp::GetHeader(std::string_view name) const {
    auto target = headers.find(name.data());
    if (target == std::end(headers)) {
        return std::nullopt;
    }
    return target->second;
}

HttpResp::Status HttpResp::GetStatus() const {
    return status_code;
}

size_t HttpResp::GetContentLength() const {
    return body.size();
}

std::string HttpResp::ToString() const {
    std::string ret;

    //  Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
    ret.append("HTTP/1.1 ");
    ret.append(std::to_string(status_code));
    ret.append(" ");
    ret.append(status_map.at(status_code));
    ret.append(CRLF);

    // headers
    // whitespace before the value is ignored
    for (auto& [name, value] : headers) {
        ret.append(name);
        ret.append(": ");  // or ":"
        ret.append(value);
        ret.append(CRLF);
    }
    // https://datatracker.ietf.org/doc/html/rfc2616#section-14.13
    ret.append("Content-Length: ");
    ret.append(std::to_string(GetContentLength()));
    ret.append(CRLF);
    // https://datatracker.ietf.org/doc/html/rfc2616#section-14.18
    ret.append("Date: ");
    ret.append(utils::DateTime::Now().ToGmtString());
    ret.append(CRLF);

    // empty line between headers and body
    ret.append(CRLF);

    // body
    ret.append(body);

    return ret;
}

}  // namespace jerry::proto::http