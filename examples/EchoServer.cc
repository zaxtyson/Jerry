//
// Created by zaxtyson on 2021/9/20.
//

#include <logger/Logger.h>
#include <net/TcpServer.h>

using namespace jerry;

class EchoServer : public net::TcpServer {
  public:
    void OnTcpConnected(net::TcpConn* conn, const DateTime& time) override {
        TcpServer::OnTcpConnected(conn, time);  // for logger
    }

    void OnTcpReceivedData(net::TcpConn* conn, const DateTime& time) override {
        auto& recv_buffer = conn->GetRecvBuffer();
        conn->Send(recv_buffer.BeginOfReadable(), recv_buffer.EndOfReadable());
        LOG_INFO("[%s] %s", conn->GetPeerAddress().GetHost().c_str(), recv_buffer.ToString().data())
        recv_buffer.DropAllBytes();
    }

    void OnTcpDisconnected(net::TcpConn* conn, const DateTime& time) override {
        TcpServer::OnTcpDisconnected(conn, time);
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

    EchoServer server;
    server.Config(config);
    server.Serve();
}