//
// Created by zaxtyson on 2021/9/25.
//

#include <net/TcpServer.h>

class FaKeNginxServer : public TcpServer {
public :
    FaKeNginxServer(EventLoop *mainLoop, const InetAddress &bindAddress, int workers) :
            TcpServer(mainLoop, bindAddress, workers) {}

    void onNewConnection(const spTcpConnection &conn) override {
//        conn->send("Hello, workers\r\n");
//
//        conn->getLoop()->runEvery(3, [conn]() {
//            if (conn->isConnected()) {
//                conn->send("Time: " + Date::now().toString() + "\n");
//            }
//        }, [conn]() { return !conn->isConnected(); });
    }

    void onReceiveMessage(const spTcpConnection &conn, MsgBuffer &buffer, Date date) override {
        std::string html = "HTTP/1.1 200 OK\r\n"
                           "Accept-Ranges: bytes\r\n"
                           "Connection: keep-alive\r\n"
                           "Content-Length: 20\r\n"
                           "Content-Type: text/html\r\n"
                           "Date: Sat, 25 Sep 2021 09:05:44 GMT\r\n"
                           "ETag: \"6108ca6f-15\"\r\n"
                           "Last-Modified: Tue, 03 Aug 2021 04:47:43 GMT\r\n"
                           "Server: nginx/1.21.1\r\n"
                           "\r\n"
                           "<h1>Hello Nginx</h1>";
        conn->send(html);
    }
};


int main() {
    EventLoop loop;
    InetAddress address("", 8081);  // OK, bind 0.0.0.0
    FaKeNginxServer server(&loop, address, 8);
    server.start();
}