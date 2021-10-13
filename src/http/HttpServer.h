//
// Created by zaxtyson on 2021/9/29.
//

#ifndef JERRY_HTTPSERVER_H
#define JERRY_HTTPSERVER_H

#include <net/TcpServer.h>
#include <http/HttpRequest.h>
#include <http/HttpResponse.h>
#include <http/HttpServlet.h>
#include <map>

class HttpServer final : public TcpServer {
public:
    using HttpServletPtr = std::shared_ptr<HttpServlet>;
public:

    HttpServer(EventLoop *mainLoop, const InetAddress &bindAddress, int workers);

    ~HttpServer() override = default;

    void registerRoute(const std::string &urlPattern, HttpServlet *servlet);

private:

    void onReceiveMessage(const spTcpConnection &conn, MsgBuffer &buffer, Date date) override;

    void onNewConnection(const spTcpConnection &conn) override;

    void onWriteComplete(const spTcpConnection &conn) override;

    void onConnectionError(const spTcpConnection &conn) override;

    void onConnectionClose(const spTcpConnection &conn) override;

public:
    void onServerClose() override;

//    const HttpServletPtr& findRoute(const std::string &path) const;

private:
    std::unordered_map<std::string, HttpServletPtr> routes_{};
};


#endif //JERRY_HTTPSERVER_H
