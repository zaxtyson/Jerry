//
// Created by zaxtyson on 2021/9/25.
//

#include <http/HttpServer.h>

using namespace jerry;

class MyHttpServer: http::HttpServer{
    void OnConnected(net::TcpConn* conn, const DateTime& time) override {
        HttpServer::OnConnected(conn, time);
    }
    void OnRequest(const http::HttpReq& req, http::HttpResp& resp, const DateTime& time) override {
        HttpServer::OnRequest(req, resp, time);
    }
    void OnDisConnected(net::TcpConn* conn, const DateTime& time) override {
        HttpServer::OnDisConnected(conn, time);
    }
};