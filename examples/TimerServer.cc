//
// Created by zaxtyson on 2022/3/21.
//

#include <logger/Logger.h>
#include <net/TcpServer.h>

using namespace jerry;

class TimerServer : public net::TcpServer {
  public:
    void OnTcpConnected(net::TcpConn* conn, const DateTime& time) override {
        TcpServer::OnTcpConnected(conn, time);
        conn->GetTimerQueue()->AddTimer(std::chrono::seconds(3), 3, [sp = conn->GetSharedPtr()] {
            char data[64]{};
            snprintf(
                data, sizeof(data), "Hello! Time now: %s\n", DateTime::Now().ToString().data());
            sp->Send(data);
        });
    }

    void OnTcpReceivedData(net::TcpConn* conn, const DateTime& time) override {
        auto& recv_buffer = conn->GetRecvBuffer();
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

    TimerServer server;
    server.Config(config);
    server.Serve();
}