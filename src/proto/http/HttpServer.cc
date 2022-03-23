//
// Created by zaxtyson on 2022/3/18.
//

#include "HttpServer.h"
#include "HttpReqDecoder.h"

namespace jerry::proto::http {

void HttpServer::OnTcpConnected(net::TcpConn* conn, const DateTime& time) {
    // set codec to parse http request
    auto decoder = std::make_shared<HttpReqDecoder>(conn->GetRecvBuffer());
    decoder->SetBodyMaxBytes(4 * 1024 * 1024);  // 4MB
    conn->SetStreamCodec(decoder);
    OnConnected(conn, time);
}

void HttpServer::OnTcpDisconnected(net::TcpConn* conn, const DateTime& time) {
    OnDisConnected(conn, time);
}

void HttpServer::OnTcpReceivedData(net::TcpConn* conn, const DateTime& time) {
    auto decoder = conn->GetDecoder<std::shared_ptr<HttpReqDecoder>>();
    auto req = decoder->Decode();
    if (req.has_value()) {
        HttpResp resp;
        OnRequest(req.value(), resp, conn, time);
        conn->Send(resp.ToString());
    }
}

void HttpServer::OnRequest([[maybe_unused]] const HttpReq& req,
                           [[maybe_unused]] HttpResp& resp,
                           [[maybe_unused]] net::TcpConn* conn,
                           [[maybe_unused]] const DateTime& time) {}

void HttpServer::OnConnected([[maybe_unused]] net::TcpConn* conn,
                             [[maybe_unused]] const DateTime& time) {}

void HttpServer::OnDisConnected([[maybe_unused]] net::TcpConn* conn,
                                [[maybe_unused]] const DateTime& time) {}

}  // namespace jerry::proto::http