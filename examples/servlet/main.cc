//
// Created by zaxtyson on 2021/9/30.
//

#include <http/HttpServer.h>
#include <utils/log/AsyncLogger.h>
#include "IndexServlet.hpp"
#include "HelloServlet.hpp"
#include "TestServlet.hpp"

int main() {
    AsyncLogger logger("./servlet.logger");
//    logger.setAutoFlushInterval(3);  // default
    Logger::setOutput(&logger);
    Logger::setLogLevel(LogLevel::kInfo);
    logger.start();

    EventLoop loop;
    InetAddress address("0.0.0.0", 8081);
    HttpServer server(&loop, address, 8);
    server.registerRoute("/", new IndexServlet());
    server.registerRoute("/hello", new HelloServlet());
    server.registerRoute("/test", new TestServlet());
    server.start();
}