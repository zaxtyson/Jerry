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
    config.acceptor.listen_ip = "127.0.0.1";
    config.acceptor.listen_port = 8080;
    config.workgroup.workers = 1;
    config.threadpool.workers = 1;
    config.threadpool.pending_size = 0;
    config.ssl.use_ssl = false;

    TimerServer server;
    server.Config(config);
    server.Serve();
}