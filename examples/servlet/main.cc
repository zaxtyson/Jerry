//
// Created by zaxtyson on 2021/9/30.
//

#include <http/HttpServer.h>
#include "HelloServlet.hpp"
#include "TestServlet.hpp"

int main() {
    EventLoop loop;
    InetAddress address("0.0.0.0", 8081);
    HttpServer server(&loop, address, 8);
    server.registerRoute("/", new HelloServlet());
    server.registerRoute("/test", new TestServlet());
    server.start();
}