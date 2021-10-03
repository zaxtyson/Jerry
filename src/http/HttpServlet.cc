//
// Created by zaxtyson on 2021/9/30.
//

#include "HttpServlet.h"

void HttpServlet::service(const HttpRequest &req, HttpResponse &resp) {
    switch (req.getMethod()) {
        case HttpRequest::Method::kGet:
            doGet(req, resp);
            break;
        case HttpRequest::Method::kPost:
            doPost(req, resp);
            break;
        case HttpRequest::Method::kPut:
            doPut(req, resp);
            break;
        case HttpRequest::Method::kDelete:
            doDelete(req, resp);
            break;
        case HttpRequest::Method::kHead:
            doHead(req, resp);
            break;
        case HttpRequest::Method::kOptions:
            doOptions(req, resp);
        default:
            doGet(req, resp);
    }
}
