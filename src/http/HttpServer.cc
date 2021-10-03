//
// Created by zaxtyson on 2021/9/29.
//

#include "HttpServer.h"
#include <http/HttpContext.h>
#include <utils/Logger.h>

HttpServer::HttpServer(EventLoop *mainLoop, const InetAddress &bindAddress, int workers) :
        TcpServer(mainLoop, bindAddress, workers) {

}

void HttpServer::onNewConnection(const spTcpConnection &conn) {
    conn->setContext(std::make_shared<HttpContext>());
}

void HttpServer::onReceiveMessage(const spTcpConnection &conn, MsgBuffer &buffer, Date date) {
    auto *ctx = reinterpret_cast<HttpContext *>(conn->getContext());
    HttpResponse response;

    if (!ctx->parseRequest(buffer)) {
        response.setStatus(HttpResponse::StatusCode::kBadRequest);
        response.getWriter()->print("<h1>400, Bad Request</h1>");
        conn->send(response.toString());
        ctx->reset();  // 重置状态, 等待下一次解析
    }

    // 没有解析失败, 但是数据不完整
    if (!ctx->isParseFinished()) return;

    // 完整解析
    HttpRequest &request = ctx->getRequest();
    LOG_DEBUG("%s %s", request.getMethodStr().c_str(), request.getRequestURI().c_str());

    // 根据路径找到 servlet 处理
    auto path = request.getServletPath();
    if (servlets_.find(path) == servlets_.end()) {
        response.setStatus(HttpResponse::StatusCode::kNotFound);
        response.getWriter()->print("<h1>404, Not Found</h1>");
    } else {
        HttpServletPtr servlet = servlets_[path];
        servlet->service(request, response);
    }

//    LOG_DEBUG("%s", response.toString().c_str());
    conn->send(response.toString());
    ctx->reset();
}

void HttpServer::onWriteComplete(const spTcpConnection &conn) {

}

void HttpServer::onConnectionError(const spTcpConnection &conn) {

}

void HttpServer::onConnectionClose(const spTcpConnection &conn) {

}

void HttpServer::registerRoute(const std::string &path, HttpServlet *servlet) {
    servlets_[path] = std::shared_ptr<HttpServlet>(servlet);
    servlet->init();
}

void HttpServer::onServerClose() {
    for (auto &servlet: servlets_) {
        servlet.second->destroy();
    }
}