//
// Created by zaxtyson on 2022/3/18.
//

#ifndef JERRY_HTTPRESP_H
#define JERRY_HTTPRESP_H

#include <map>
#include <string>
#include <string_view>

namespace jerry::proto::http {

class HttpResp {
  public:
    using Headers = std::multimap<std::string, std::string>;
    using Status = uint16_t;
    using Body = std::string;

  public:
    void SetStatus(Status status_code);
    void SetContentType(std::string_view content_type);
    void SetServer(std::string_view server_name);
    void SetKeepAlive();
    void AddEtag(std::string_view etag);
    void AddHeader(std::string_view name, std::string_view value, bool multiple = false);
    void SetRedirect(std::string_view location, Status status_code = 302);
    void SetBody(std::string_view body);
    void AppendBody(std::string_view body);

  public:
    std::optional<std::string_view> GetHeader(std::string_view name) const;
    Status GetStatus() const;
    size_t GetContentLength() const;
    std::string ToString() const;

  private:
    Headers headers{};
    Status status_code{200};
    Body body{};

  private:
    static const std::map<Status, std::string> status_map;
};

}  // namespace jerry::http


#endif  // JERRY_HTTPRESP_H
