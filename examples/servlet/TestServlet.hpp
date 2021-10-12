//
// Created by zaxtyson on 2021/9/30.
//

#include <http/HttpServlet.h>
#include <utils/log/Logger.h>

class TestServlet : public HttpServlet {
public:
    void init() override {
        LOG_INFO("TestServlet init called");
    }

    void doGet(const HttpRequest &req, HttpResponse &resp) override {
        if (req.getParameter("redirect") == "true") {
            resp.sendRedirect(req.getParameter("url"));
        } else if (!req.getParameter("name").empty()) {
            resp.getWriter()->print("<h1> Hello, " + req.getParameter("name") + "</h1>");
        } else {
            resp.getWriter()->print("<h1>Test Page~</h1>");
        }
    }

    void doPost(const HttpRequest &req, HttpResponse &resp) override {
        HttpServlet::doPost(req, resp);
    }
};