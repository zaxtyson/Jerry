//
// Created by zaxtyson on 2022/3/22.
//

#include "WebsocketServer.h"
#include "WebsocketCodec.h"

namespace jerry::proto::ws {

void WebsocketServer::OnTcpConnected(net::TcpConn* conn, const DateTime& time) {
    // set codec to parse websocket packet
    auto codec = std::make_shared<WebsocketCodec>(conn);
    conn->SetStreamCodec(codec);
    OnConnected(conn, time);
}

void WebsocketServer::OnTcpDisconnected(net::TcpConn* conn, const DateTime& time) {
    OnDisConnected(conn, time);
}

void WebsocketServer::OnTcpReceivedData(net::TcpConn* conn, const DateTime& time) {
    auto codec = conn->GetDecoder<std::shared_ptr<WebsocketCodec>>();
    auto req = codec->Decode();
    if (req.has_value()) {
        WsResp resp;
        OnRequest(req.value(), resp, conn, time);

        // encode frame and send to client
        conn->Send(codec->EncodeFrameHead(resp.GetPayload()));
        conn->Send(resp.GetPayload());
    }
}

void WebsocketServer::OnConnected(net::TcpConn* conn, const DateTime& time) {}

void WebsocketServer::OnRequest(const WsReq& req,
                                WsResp& resp,
                                net::TcpConn* conn,
                                const DateTime& time) {}

void WebsocketServer::OnDisConnected(net::TcpConn* conn, const DateTime& time) {}
}  // namespace jerry::proto::ws