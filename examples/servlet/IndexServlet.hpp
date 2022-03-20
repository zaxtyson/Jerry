//
// Created by zaxtyson on 2021/9/30.
//

#include <http/HttpServlet.h>
#include <utils/log/Logger.h>
#include <iostream>
#include <fstream>

class IndexServlet : public HttpServlet {
private:
    const std::string basePath{"./webroot"};
public:
    void init() override {
        LOG_INFO("IndexServlet init");
    }

    void destroy() override {
        LOG_INFO("IndexServlet destroy");
    }

    std::string getFileContent(const std::string &filePath) {
        std::string fullPath = basePath + filePath;
        if (filePath == "/") {
            fullPath += "index.html";
        }
        std::ifstream file(fullPath, std::ifstream::in);
        if (file.is_open()) {
            return {(std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()};
        } else {
            LOG_ERROR("File not found: %s", fullPath.c_str());
        }
        return "";
    }

    void doGet(const HttpRequest &req, HttpResponse &resp) override {
        PrintWriter *out = resp.getWriter();
        std::string fileContent = getFileContent(req.getRequestURI());
        if (fileContent.empty()) {
            resp.setStatus(HttpResponse::StatusCode::kNotFound);
            out->print("<h1>404 Not Found</h1>");
        } else {
            out->print(fileContent);
        }
    }

    void doPost(const HttpRequest &req, HttpResponse &resp) override {
        doGet(req, resp);
    }
};