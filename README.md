# JERRY

## 项目介绍

本项目是参考 Muduo 实现的高性能网络服务器框架, 参考 Tomcat 实现了 Servlet 
风格的接口(所以项目叫Jerry), 封装了一些工具类方便编程使用 

## 项目结构

```
.
├── docs              # 文档
├── examples      　　 # 示例代码
├── src
│     ├── db　   　　   # 数据库相关
│     ├── http         # Http/Servlet 相关
│     ├── net          # 网络基础库
│     └── utils　　     # 工具类
└── unittests          # 单元测试
```

## 如何编译

本项目使用 Cmake 进行编译, 使用 catch2 进行单元测试, 使用 wrk 进行压测

```
git clone https://github.com.cnpmjs.org/zaxtyson/Jerry.git
cd Jerry
```

在项目根目录的 `CMakeLists.txt` 配置编译参数, 如日志/是否编译example/是否编译单元测试等等

```
mkdir build
cmake ..
make -j
```

编译生成的二进制程序在 `build/examples` 和 `build/unittests` 下


## 技术点

- 非阻塞式IO, 使用 epoll 实现 IO 多路复用, LT 模式触发
- 主从 Reactor 模型, 主线程负责 Accept, 通过 RoundRobin 分发给 Reactor 线程池处理
- 支持长连接、优雅关闭, 发送完缓冲区才 shutdown
- 使用向量 IO 实现高性能缓冲区
- 使用 eventfd 实现线程间的异步唤醒
- 使用小根堆实现了定时器, 支持单次定时/重复定时(可设置条件自动停止)
- 使用 timerfd 实现触发最近一次超时的定时器
- 使用状态机解析 HTTP 请求
- 封装 Servlet 接口, 方便编写 Web 程序

## 测试

see docs

## TODO

- [ ] 高性能日志
- [ ] MySQL 连接池
- [ ] HTTP Sessions
- [ ] Servlet Filter
- [ ] SSL/TLS 支持

## 折腾过程

本来打算使用 ET 模式读取数据, 期望通过减少系统调用次数来提高效率。后来看到了 Muddo Buffer 的巧妙设计，大呼妙啊:
初始只给 Buffer 分配 1024B 空间, 通过 scatter/gather IO 把多余的数据写到栈上的 extraBuffer(65535B),
一次 readv 调用就可以应付大部分场景的读取操作, 读取完毕我们再栈上的 extraBuffer 追加到堆上的 Buffer, 实现自适应扩容。
这么做不但避免了内存的浪费, 也防止了 ET 模式下处理某些数据量大的连接时耽搁太久, 导致拖慢对其它连接的响应。

muduo 框架中大量使用 bind 回调，从设计模式上来上，"组合优于继承" 确实更加灵活，而且耦合性更低。但是每次用户的类要使用 TcpServer,
就得重复写一堆回调函数然后注册，似乎很繁琐。我不喜欢重复出现的代码，因此从 TcpServer 开始我就把这条回调链打断了，把 TcpServer
作为了基类，自它之后就开始用 OOP 那套机制，子类重写需要的接口就好了，就像 Servlet 那样。
当然虚函数对动态库的二进制兼容性的是有影响的，如果后期要升级库，虚拟函数表的变化，
可能导原有的代码出问题，这方面陈硕大大自然有他的考量。纠结了很久，我还是打算这样设计，主要是为了自己用着方便

性能分析时发现，各个 IO 线程事件循环 75% 左右的时间是在处理回调(任务分配的比较均匀)，10~15% 左右的时间是在读 fd。
在回调的这部分时间中, 24% 左右耗在 Log 上面了, 因为我是直接输出到 std::out, 开销这么大可以理解。但是让我意外的是，
我为了观察 Epoll 触发事件类型的函数 getEventTypeName 居然吃掉了 12% 的性能！只是因为每次
Epoll 触发它都要构造一个 stringstream 对象，后面换成普通的 string 之后, 性能开销直接降到了 1.3% 左右! 
然而我的 Log 还是要吃掉 1/3 的时间，关掉日志后用 wrk 测压，服务器吞吐量翻了近 7 倍！
