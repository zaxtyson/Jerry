//
// Created by zaxtyson on 2021/9/25.
//

#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <catch2/catch.hpp>
#include <iostream>
#include <net/BaseBuffer.h>

using std::cout;
using std::endl;
using namespace jerry::net;

SCENARIO("BaseBuffer Test") {
    GIVEN("A empty Buffer") {
        BaseBuffer buffer(1024);
        REQUIRE(buffer.ReadableBytes() == 0);
        REQUIRE(buffer.WritableBytes() == 1024);
        REQUIRE(buffer.ToStringView().empty());

        WHEN("append some data to the buffer") {
            std::string_view s1{"Hello World!"};
            std::string_view s2{"Tom cat and Jerry"};
            buffer.Append(s1);
            buffer.Append(s2);

            THEN("check buffer") {
                std::string_view s3{"Hello World!Tom cat and Jerry"};
                std::string_view s4{buffer.BeginOfReadable().base(), buffer.ReadableBytes()};
                REQUIRE(buffer.ReadableBytes() == s1.size() + s2.size());
                REQUIRE(buffer.WritableBytes() == 1024 - (s1.size() + s2.size()));
                REQUIRE(buffer.ToStringView() == s3);
                REQUIRE(s3 == s4);
            }

            THEN("drop some data in the buffer") {
                buffer.DropBytes(4);
                REQUIRE(buffer.ReadableBytes() == s1.size() + s2.size() - 4);
                REQUIRE(buffer.WritableBytes() == 1024 - (s1.size() + s2.size()));
                REQUIRE(buffer.ToStringView() == "o World!Tom cat and Jerry");
            }

            THEN("drop some data in the buffer") {
                std::string_view target{"cat"};
                auto iter = std::search(
                    buffer.BeginOfReadable(), buffer.EndOfReadable(), target.begin(), target.end());
                REQUIRE(iter != buffer.EndOfReadable());
                REQUIRE(*iter == 'c');
                buffer.DropBytesUntil(iter);
                REQUIRE(buffer.ReadableBytes() == 13);
                REQUIRE(buffer.WritableBytes() == 1024 - (s1.size() + s2.size()));
                REQUIRE(buffer.ToStringView() == "cat and Jerry");
            }

            THEN("drop all data in the buffer") {
                buffer.DropAllBytes();
                REQUIRE(buffer.ReadableBytes() == 0);
                REQUIRE(buffer.WritableBytes() == 1024);
                REQUIRE(buffer.ToStringView().empty());
            }
        }

        WHEN("Read from a fd") {
            int fd = open("/dev/zero", O_RDONLY);
            REQUIRE(fd > 0);

            REQUIRE(buffer.ReadableBytes() == 0);
            REQUIRE(buffer.WritableBytes() == 1024);

            auto n = buffer.ReadBytesFromFd(fd);
            REQUIRE(n == 1024 + 65535);  // buffer + extra_buffer
            REQUIRE(buffer.ReadableBytes() == 1024 + 65535);
            REQUIRE(buffer.WritableBytes() == 0);

            buffer.DropBytes(10);
            REQUIRE(buffer.ReadableBytes() == 1024 + 65535 - 10);
            REQUIRE(buffer.WritableBytes() == 0);

            buffer.Append("abc");
            REQUIRE(buffer.ReadableBytes() == 1024 + 65535 - 10 + 3);
            REQUIRE(buffer.WritableBytes() == 7);

            close(fd);
        }
    }
}