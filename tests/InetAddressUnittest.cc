//
// Created by zaxtyson on 2021/9/19.
//

#include "catch2.hpp"
#include "net_old/InetAddress2.h"

SCENARIO("InetAddress2 Test") {
    GIVEN("A normal address") {
        InetAddress addr("127.0.0.1", 8080);
        REQUIRE(addr.getIp() == "127.0.0.1");
        REQUIRE(addr.getPort() == 8080);
        REQUIRE(addr.getIpPort() == "127.0.0.1:8080");
    }

    GIVEN("A address without ip") {
        InetAddress addr("", 8080);
        REQUIRE(addr.getIp() == "0.0.0.0");
        REQUIRE(addr.getIpPort() == "0.0.0.0:8080");
    }

    GIVEN("A address with invalid port") {
        WARN("port cannot <=0 or >= 65535");
        InetAddress addr("192.168.1.1", 0);
        REQUIRE(addr.getPort() == 80);
    }
}