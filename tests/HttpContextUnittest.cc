//
// Created by zaxtyson on 2021/10/1.
//

#include "catch2.hpp"
#include <net/MsgBuffer.h>
#include <http/HttpContext.h>
#include <utils/log/Logger.h>

SCENARIO("Test HttpContext") {
    GIVEN("HttpContext and Buffer") {
        MsgBuffer buffer;
        HttpContext context;

        WHEN("parse normal GET request") {
            buffer.append("GET /foo/bar/index.html HTTP/1.1\r\n"
                          "Accept-Encoding: gzip, deflate\r\n"
                          "Connection: keep-alive\r\n"
                          "Host: 127.0.0.1:8081\r\n"
                          "User-Agent: HTTPie/2.5.0\r\n"
                          "\r\n");

            REQUIRE(context.parseRequest(buffer));
            REQUIRE(context.isParseFinished());
            HttpRequest &req = context.getRequest();
            REQUIRE(req.getMethod() == HttpRequest::Method::kGet);
            REQUIRE(req.getVersion() == HttpRequest::Version::kHttp11);
            REQUIRE(req.getRequestURI() == "/foo/bar/index.html");
            REQUIRE(req.getServletPath() == "/foo/bar");
            REQUIRE(req.getHeader("Host") == "127.0.0.1:8081");
            REQUIRE(req.getHeader("User-Agent") == "HTTPie/2.5.0");
            REQUIRE(req.getHeader("Connection") == "keep-alive");
        }

        WHEN("parse GET request with parameters1") {
            buffer.append("GET /index.html? HTTP/1.1\r\n"
                          "\r\n");

            REQUIRE(context.parseRequest(buffer));
            REQUIRE(context.isParseFinished());
            HttpRequest &req = context.getRequest();
            REQUIRE(req.getServletPath() == "/");
            REQUIRE(req.getQueryString() == "");
        }

        WHEN("parse GET request with parameters2") {
            buffer.append("GET /index.html?foo=bar HTTP/1.1\r\n"
                          "\r\n");

            REQUIRE(context.parseRequest(buffer));
            REQUIRE(context.isParseFinished());
            HttpRequest &req = context.getRequest();
            REQUIRE(req.getServletPath() == "/");
            REQUIRE(req.getQueryString() == "foo=bar");
            REQUIRE(req.getParameter("foo") == "bar");
        }

        WHEN("parse GET request with parameters3") {
            buffer.append("GET /foo/bar/index.html?foo=bar&blank1=&key=value&blank2=#frag HTTP/1.1\r\n"
                          "\r\n");

            REQUIRE(context.parseRequest(buffer));
            REQUIRE(context.isParseFinished());
            HttpRequest &req = context.getRequest();
            REQUIRE(req.getServletPath() == "/foo/bar");
            REQUIRE(req.getQueryString() == "foo=bar&blank1=&key=value&blank2=");
            REQUIRE(req.getParameter("foo") == "bar");
            REQUIRE(req.getParameter("key") == "value");
            REQUIRE(req.getParameter("blank1") == "");
            REQUIRE(req.getParameter("blank2") == "");
            REQUIRE(req.getParameter("not_this_filed") == "");
        }

        WHEN("parse non-continuous GET request") {
            buffer.append("GET /foo/bar/index.html?foo=bar&key=value#fragid1 HTTP/1.1\r\n"
                          "Connection: ");

            REQUIRE(context.parseRequest(buffer));
            REQUIRE_FALSE(context.isParseFinished());

            buffer.append("keep-alive\r\n");
            REQUIRE(context.parseRequest(buffer));
            REQUIRE_FALSE(context.isParseFinished());

            buffer.append("\r\nhello");
            REQUIRE(context.parseRequest(buffer));
            REQUIRE(context.isParseFinished());
            REQUIRE(context.getRequest().getBody() == "hello");
        }

        WHEN("parse bad GET request") {
            buffer.append("GET / HTTP/1.1\r\n");

            REQUIRE(context.parseRequest(buffer));
            REQUIRE_FALSE(context.isParseFinished());

            buffer.append("keep-alive\r\n");
            REQUIRE_FALSE(context.parseRequest(buffer));
            REQUIRE_FALSE(context.isParseFinished());
        }


    }

}