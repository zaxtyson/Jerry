<div align="center">
    <img src="docs/images/jerry_logo.png" width="150" alt="jerry_logo">
    <h1> ≡ Jerry ≡ </h1>
</div>

## 简介

- ✨ Jerry 是一个高性能的 C++ 网络库
- 😎 内置定时器、线程池、异步日志等组件, 方便使用
- 🛠 支持编写 Encoder/Decoder 处理自定义协议
- ♻ 基于 Reactor 模型, 使用 Epoll 驱动事件循环
- ⚡ 使用 Kernel 3.9+ 提供的 `REUSEPORT` 特性实现高效的负载均衡
- ❤️ 使用 Modern C++ 开发, 对人类友好

## 编译

```
git clone -b refactor --recurse-submodules --shallow-submodules https://github.com/zaxtyson/Jerry.git
```

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

## TODO

- [ ] TLS 支持
- [ ] WebSocket 支持
- [ ] MySQL 连接池
- [ ] 协程支持