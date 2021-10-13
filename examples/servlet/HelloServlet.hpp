//
// Created by zaxtyson on 2021/9/30.
//

#include <http/HttpServlet.h>
#include <utils/log/Logger.h>

class HelloServlet : public HttpServlet {
public:
    void init() override {
        LOG_INFO("HelloServlet init");
    }

    void destroy() override {
        LOG_INFO("HelloServlet destroy");
    }

    void doGet(const HttpRequest &req, HttpResponse &resp) override {
        resp.addHeader("key", "value");
        PrintWriter *out = resp.getWriter();
        out->print(R"(
        <html>
            <head>
                <title>HelloServlet</title>
            </head>
            <body>
                <h1>Hello World</h1>
            </body>
        </html>
        )");
    }

    void doPost(const HttpRequest &req, HttpResponse &resp) override {
        doGet(req, resp);
    }
};