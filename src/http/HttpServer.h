//
// Created by zaxtyson on 2022/3/18.
//

#ifndef JERRY_HTTPSERVER_H
#define JERRY_HTTPSERVER_H

#include "HttpReq.h"
#include "HttpResp.h"
#include "net/TcpServer.h"

namespace jerry::http {

class HttpServer : public net::TcpServer {
  public:
    HttpServer() = default;
    ~HttpServer() override = default;

    virtual void OnConnected(net::TcpConn* conn, const DateTime& time);
    virtual void OnRequest(const HttpReq& req,
                           HttpResp& resp,
                           net::TcpConn* conn,
                           const DateTime& time);
    virtual void OnDisConnected(net::TcpConn* conn, const DateTime& time);

  private:
    void OnTcpConnected(net::TcpConn* conn, const DateTime& time) override;
    void OnTcpDisconnected(net::TcpConn* conn, const DateTime& time) override;
    void OnTcpReceivedData(net::TcpConn* conn, const DateTime& time) override;
};
}  // namespace jerry::http


#endif  // JERRY_HTTPSERVER_H
