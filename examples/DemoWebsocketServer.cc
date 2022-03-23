//
// Created by zaxtyson on 2022/3/22.
//

#include <logger/Logger.h>
#include <proto/ws/WebsocketServer.h>

using namespace jerry;
using namespace jerry::proto::ws;

class DemoWebsocketServer : public WebsocketServer {
  public:
    void OnConnected(jerry::net::TcpConn* conn, const jerry::DateTime& time) override {
        WebsocketServer::OnConnected(conn, time);
    }

    void OnRequest(const WsReq& req,
                   WsResp& resp,
                   net::TcpConn* conn,
                   const DateTime& time) override {
        LOG_INFO("[%s] [%zu] %s",
                 req.GetRequestUri().data(),
                 req.GetPayloadLength(),
                 req.GetPayload().data())

        if (req.GetFrameType() == WsFrameType::kPing) {
            // no pyload to send
            resp.SetFrameType(WsFrameType::kPong);
        } else {
            // default frame type is `WsFrameType::kText`
            resp.AppendData("Received: ");
            resp.AppendData(req.GetPayload());
        }
    }

    void OnDisConnected(jerry::net::TcpConn* conn, const jerry::DateTime& time) override {
        WebsocketServer::OnDisConnected(conn, time);
    }
};

int main() {
    Logger::SetAppender(new logger::StderrAppender());
    Logger::SetLogLevel(LogLevel::kDebug);

    net::ServerConfig config;
    config.listen_ip = "127.0.0.1";
    config.listen_port = 8080;
    config.event_loop_size = 1;
    config.thread_pool_size = 1;
    config.thread_pool_pending = 0;

    DemoWebsocketServer server;
    server.Config(config);
    server.Serve();

    // you can test websocket here:
    // http://livepersoninc.github.io/ws-test-page/
}