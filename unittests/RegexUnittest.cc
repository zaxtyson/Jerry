//
// Created by zaxtyson on 2021/9/30.
//

#include "catch2.hpp"
#include <regex>
#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono_literals;
using std::cout;
using std::endl;

//    GET /index.html HTTP/1.1
//    Accept: */*
//    Accept-Encoding: gzip, deflate
//    Connection: keep-alive
//    Host: 127.0.0.1
//    User-Agent: HTTPie/2.5.0

SCENARIO("Parse HTTP request test") {
    GIVEN("Http request line") {
        auto request = "GET /path/to/index.html HTTP/1.1";

        WHEN("Parse with regex") {
            // 匹配 10w 次, 主要开销在匹配 path, 只提取, 不检查合规性
            // 不要使用贪婪模式匹配, 否则匹配空格时状态机回溯，开销是非贪婪模式的 2 倍
//            auto reg = std::regex(R"((GET|POST) (/[\w/?=%&.-_]+?) HTTP/1.(\d))"); // 550-590ms
            auto reg = std::regex(R"((GET|POST) (/.+?) HTTP/1.(\d))"); // 540-570ms
//            auto reg = std::regex(R"((.+?) (.+?) (.+?))"); // 640-680ms
//            auto reg = std::regex(R"((.+) (.+) (.+))"); // 1100-1200ms

            auto start = std::chrono::high_resolution_clock::now();

            std::cmatch cm;
            for (int i = 0; i < 1e5; i++) {
                std::regex_match(request, cm, reg);
//                std::cout << cm[1] << " " << cm[2] << " " << cm[3] << " " << cm[4] << std::endl;
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            cout << "Match result: " << cm.size() << ", regex parse cost: " << ms << "ms" << endl;
        }

        WHEN("Parse with std::search") {
            std::string method, path, version;

            auto start = std::chrono::high_resolution_clock::now();

            for (int i = 0; i < 1e5; i++) {
                auto begin = request;
                auto end = request + strlen(request);

                auto space1 = std::find(begin, end, ' ');
                method = std::string(begin, space1);

                auto space2 = std::find(space1 + 1, end, ' ');
                path = std::string(space1 + 1, space2);

                version = std::string(end - 1, end);
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            cout << method << "|" << path << "|" << version
                 << "\tstd::search parse cost: " << ms << "ms" << endl << endl;   // 18ms
        }
    }

    GIVEN("Http request Header") {
        auto request = "Accept-Encoding: gzip, deflate";

        WHEN("Parse with regex") {
            std::regex reg(R"((.+?): (.+?))");

            auto start = std::chrono::high_resolution_clock::now();

            std::cmatch cm;
            for (int i = 0; i < 1e5; i++) {
                std::regex_match(request, cm, reg);
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            cout << "Match result: " << cm.size() << ", regex parse cost: " << ms << "ms" << endl;
        }

        WHEN("Parse with std::search") {
            std::string key, value;

            auto start = std::chrono::high_resolution_clock::now();

            for (int i = 0; i < 1e5; i++) {
                auto begin = request;
                auto end = request + strlen(request);

                auto delimiter = std::find(begin, end, ':');
                key = std::string(begin, delimiter);
                value = std::string(delimiter + 2, end);
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            cout << key << "|" << value
                 << "\tstd::search parse cost: " << ms << "ms" << endl;   // 11ms
        }
    }
}
