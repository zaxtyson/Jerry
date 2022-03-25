//
// Created by zaxtyson on 2021/9/25.
//

#include <logger/Logger.h>
#include <proto/http/HttpServer.h>

using namespace jerry;
using namespace jerry::proto::http;
using namespace std::chrono_literals;

class DemoHttpServer : public HttpServer {
    void OnConnected(net::TcpConn* conn, const DateTime& time) override {
        LOG_INFO(
            "%s connected at %s", conn->GetPeerAddress().GetHost().data(), time.ToString().data())
    }

    void OnRequest(const HttpReq& req,
                   HttpResp& resp,
                   net::TcpConn* conn,
                   const DateTime& time) override {
        LOG_INFO("%s | %s %s %s | %s | body %zu bytes",
                 conn->GetPeerAddress().GetHost().data(),
                 req.GetMethodString().data(),
                 req.GetRequestUri().data(),
                 req.GetVersionString().data(),
                 time.ToString().data(),
                 req.GetBody().size())
        LOG_INFO("%s", req.GetHeader("User-Agent").value_or("no user-agent").data())

        resp.SetServer("Jerry/0.5");
        resp.SetContentType("text/html; charset=utf-8");
        char buf[8192]{};
        auto last_time = conn->GetContext<DateTime>("last_time").value_or(DateTime::Now());
        snprintf(buf,
                 sizeof(buf),
                 R"(
                    <html>
                        <head>
                            <title>Demo</title>
                        </head>
                        <body>
                            <h1>Hello, This is Jerry!</h1>
                            <p>Server received request: %s</p>
                            <p>Last visit time: %s</p>
                            <hr>
                            <div>
                                <h4>Your Request</h4>
                                <pre>%s</pre>
                            </div>
                        </body>
                    </html>
                )",
                 time.ToString().data(),
                 last_time.ToString().data(),
                 req.ToString().data());

        resp.SetBody(buf);
        conn->SetContext("last_time", time);
    }

    void OnDisConnected(net::TcpConn* conn, const DateTime& time) override {
        LOG_INFO("%s disconnected at %s",
                 conn->GetPeerAddress().GetHost().data(),
                 time.ToString().data())
    }

    void Monitor() override {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        printf("[Health Check] Total connections: %zu, Total timers: %zu, Pending tasks: %zu\n",
               GetTotalConns(),
               GetTotalTimers(),
               GetPendingTasks());
    }
};

int main() {
    auto* appender = new logger::AsyncFileAppender("app.log");
    appender->Start();  // start an async logger thread
    Logger::SetAppender(appender);
    Logger::SetLogLevel(LogLevel::kInfo);

    net::ServerConfig config;
    config.acceptor.listen_ip = "127.0.0.1";
    config.acceptor.listen_port = 8443;
    config.workgroup.workers = 1;
    config.threadpool.workers = 0;
    config.threadpool.pending_size = 0;

    // to generate certification: https://mkcert.dev/
    // mkcert -key-file key.pem -cert-file cert.pem localhost 127.0.0.1 ::1
    config.ssl.use_ssl = true;
    config.ssl.disable_old_ssl_version = true;
    config.ssl.enable_validation = true;
    config.ssl.cert_file = "./pem/cert.pem";
    config.ssl.private_key_file = "./pem/key.pem";

    DemoHttpServer server;
    server.Config(config);
    server.Serve();

    appender->Stop();
}