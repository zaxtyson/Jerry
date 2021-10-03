//
// Created by zaxtyson on 2021/9/25.
//

#include "catch2.hpp"
#include <iostream>
#include <net/MsgBuffer.h>
#include <sys/types.h>
#include <fcntl.h>

using std::cout;
using std::endl;

SCENARIO("MsgBuffer test") {
GIVEN("A blank MsgBuffer") {
MsgBuffer buffer;

REQUIRE(buffer
.

capacity()

== 1024 + buffer.

prependableBytes()

);
REQUIRE(buffer
.

readableBytes()

== 0);

WHEN("append some data to the buffer") {
buffer.
append("Hello, world!");
buffer.append("yyds");


THEN("check is buffer starts with str") {
REQUIRE(buffer
.startsWith("Hello"));
REQUIRE_FALSE(buffer
.startsWith("hello"));
}

THEN("search string in buffer") {
auto pos = buffer.search("y");
REQUIRE(std::string(pos, pos + 4)
== "yyds");
auto npos = buffer.search("abc");
REQUIRE(npos
== buffer.

readEnd()

);
}

THEN("drop some data from the buffer") {
size_t rawLength = buffer.readableBytes();
buffer.drop(5);
REQUIRE(buffer
.

readableBytes()

== rawLength - 5);
auto pos = buffer.search("!");
buffer.
dropUntil(pos);
REQUIRE(buffer
.

popAll()

== "!yyds");
}

THEN("read some data from the buffer") {
// 此处修改并不会影响下一个测试
REQUIRE(buffer
.pop(5) == "Hello");
REQUIRE(buffer
.pop(1) == ",");
REQUIRE(buffer
.pop(0) == "");
}

THEN("read all data from the buffer") {
REQUIRE(buffer
.

popAll()

== "Hello, world!yyds");
REQUIRE(buffer
.

readableBytes()

== 0);
REQUIRE(buffer
.

popAll()

== "");
}
}
}

GIVEN("A file descriptor") {
int fd = open("/dev/zero", O_RDONLY);
MsgBuffer buffer;

WHEN("read data from fd first time") {
REQUIRE(buffer
.

readableBytes()

== 0);
ssize_t n = buffer.readFromFd(fd, nullptr);
REQUIRE(n
== 1024 + 65535);  // vector + extraBuffer size
REQUIRE(buffer
.

readableBytes()

== 65535 + 1024);
REQUIRE(buffer
.

capacity()

== 65535 + 1024 + buffer.

prependableBytes()

); // 预先保留些空间
}

WHEN("read more data from fd") {
// 自适应扩容, x2
cout << "Buffer capacity now: " << buffer.

capacity()

<<
endl;
buffer.
readFromFd(fd,
nullptr);
cout << "Buffer capacity now: " << buffer.

capacity()

<<
endl;
buffer.
readFromFd(fd,
nullptr);
cout << "Buffer capacity now: " << buffer.

capacity()

<<
endl;
close(fd);
}
}
}