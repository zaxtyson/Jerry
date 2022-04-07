<div align="center">
    <img src="docs/images/jerry_logo.png" width="150" alt="jerry_logo">
    <h1> ≡ Jerry ≡ </h1>
</div>

## 简介

- ✨ Jerry 是一个高性能的 C++ 网络库
- 😎 内置定时器(支持单次/重复/条件定时)、线程池(支持优先级)、异步日志等组件
- ♻ 基于 Reactor 模型, 使用 Epoll 驱动事件循环
- 🛠 支持 HTTP/Websocket, 可编写 Codec 处理自定义协议
- 🛡 使用 OpenSSL 提供 SSL/TLS 流量加密(可选)
- ⚡ 使用 Kernel 3.9+ 提供的 `REUSEPORT` 特性实现高效的负载均衡
- ❤️ 使用 Modern C++ 开发, 对人类友好


## 总体架构

![jerry-structure](docs/images/jerry_structure.svg)


## 编译

本项目基于 C++17 开发, 如果编译器版本过低, 请[升级编译器工具链](docs/update-dev-tools.md)

```
git clone --recurse-submodules --shallow-submodules https://github.com/zaxtyson/Jerry.git
```

需要修改编译选项请查看 [CMakeLists.txt](CMakeLists.txt)

```
mkdir build
cd build
cmake ..
make
```

编译产物位于 `Jerry/dist`


## 示例

- [EchoServer](examples/EchoServer.cc)
- [HttpServer](examples/DemoHttpServer.cc)
- [TimerServer](examples/TimerServer.cc)
- [WebsocketServer](examples/DemoWebsocketServer.cc)


## TODO

- [ ] MySQL/Redis 连接池
- [ ] 协程支持
- [ ] 自定义限流器
- [ ] io_uring