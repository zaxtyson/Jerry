//
// Created by zaxtyson on 2022/3/22.
//

#ifndef JERRY_WEBSOCKETSERVER_H
#define JERRY_WEBSOCKETSERVER_H

#include "WsReq.h"
#include "WsResp.h"
#include "net/TcpServer.h"

namespace jerry::proto::ws {

class WebsocketServer : public net::TcpServer {
  public:
    WebsocketServer() = default;
    ~WebsocketServer() override = default;

    virtual void OnConnected(net::TcpConn* conn, const DateTime& time);
    virtual void OnRequest(const WsReq& req,
                           WsResp& resp,
                           net::TcpConn* conn,
                           const DateTime& time);
    virtual void OnDisConnected(net::TcpConn* conn, const DateTime& time);

  private:
    void OnTcpConnected(net::TcpConn* conn, const DateTime& time) override;
    void OnTcpDisconnected(net::TcpConn* conn, const DateTime& time) override;
    void OnTcpReceivedData(net::TcpConn* conn, const DateTime& time) override;
};
}  // namespace jerry::proto::ws

#endif  // JERRY_WEBSOCKETSERVER_H
