//
// Created by zaxtyson on 2021/9/20.
//

#include <net/TcpServer.h>
#include <utils/Logger.h>
#include <thread>

class EchoServer : public TcpServer {
private:
    std::string data{};
public:
    EchoServer(EventLoop *mainLoop, const InetAddress &bindAddress, int workers) : TcpServer(mainLoop, bindAddress,
                                                                                             workers) {}

    void onReceiveMessage(const spTcpConnection &conn, MsgBuffer &buffer, Date date) override {
        data = buffer.popAll();
        LOG_INFO("Get message from %s: %s", conn->getPeerAddress().getIpPort().c_str(), data.c_str());
        conn->send(data);
    }
};

int main() {
    EventLoop loop;
    InetAddress address("127.0.0.1", 8080);
    EchoServer server(&loop, address, 4);
    server.start();
}