//
// Created by zaxtyson on 2021/9/30.
//

#ifndef JERRY_HTTPSERVLET_H
#define JERRY_HTTPSERVLET_H

#include <http/HttpRequest.h>
#include <http/HttpResponse.h>

class HttpServlet {
public:
    HttpServlet() = default;

    virtual ~HttpServlet() = default;

    virtual void init() {}

    virtual void destroy() {}

    virtual void service(const HttpRequest &request, HttpResponse &resp);

    virtual void doGet(const HttpRequest &req, HttpResponse &resp) {}

    virtual void doPost(const HttpRequest &req, HttpResponse &resp) {}

    virtual void doPut(const HttpRequest &req, HttpResponse &resp) {}

    virtual void doDelete(const HttpRequest &req, HttpResponse &resp) {}

    virtual void doHead(const HttpRequest &req, HttpResponse &resp) {}

    virtual void doOptions(const HttpRequest &req, HttpResponse &resp) {}

};


#endif //JERRY_HTTPSERVLET_H
