//
// Created by zaxtyson on 2021/9/20.
//

#include <net/TcpServer.h>
#include <utils/log/Logger.h>
#include <utils/SockOption.h>
#include <signal.h>
#include <cstring>

TcpServer::TcpServer(EventLoop *mainLoop, const InetAddress &bindAddress, int workers) :
        mainLoop_(mainLoop), eventLoopThreadPool_(mainLoop, workers) {

    // 当客户端关闭连接, 我们还在读或者写时, 会触发 SIGPIPE
    // 默认会导致程序关闭, 这里要忽略它
    // https://stackoverflow.com/questions/21687695/getting-sigpipe-with-non-blocking-sockets-is-this-normal
    signal(SIGPIPE, SIG_IGN);

    // TcpServer 创建时, Acceptor 绑定地址并开始监听
    ::setNonBlockAndCloseOnExec(acceptor_.getFd());
    ::setReuseAddr(acceptor_.getFd());
    acceptor_.bindAddress(bindAddress);
    acceptor_.listen();

    // 绑定 Acceptor 的触发事件, TcpServer启动时把它加入 Poller
    acceptorChannel_.setFd(acceptor_.getFd());
    acceptorChannel_.enableReading();
    acceptorChannel_.setReadCallback([this](Date _) { onNewConnection(); });
    acceptorChannel_.setErrorCallback([this] { onConnectionError(); });
}


void TcpServer::onNewConnection() {
    InetAddress peerAddress;
    int clientFd = acceptor_.accept(peerAddress);
    if (clientFd < 0) {
        // 保留空闲的 fd 以应对 fd 达到系统限制的情况
        if (errno == EMFILE) {
            close(idleFd_);
            idleFd_ = acceptor_.accept(peerAddress);
            close(idleFd_);
            idleFd_ = open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
        LOG_WARN("Accept new client failed: %s", strerror(errno));
        return;
    }

    LOG_DEBUG("New client connected: %s", peerAddress.getIpPort().c_str());
    EventLoop *loop = eventLoopThreadPool_.getNextLoop();  // 给新来的客户端分配一个 EventLoop

    // 为新来的客户端创建一个 TcpConnection, 绑定相关回调函数
    spTcpConnection conn = std::make_shared<TcpConnection>(loop, clientFd, acceptor_.getLocalAddress(), peerAddress);
    conn->setWriteCompleteCallback([this](const spTcpConnection &conn) { onWriteComplete(conn); });
    conn->setCloseCallback([this](const spTcpConnection &conn) { onConnectionClose(conn); });
    conn->setErrorCallback([this](const spTcpConnection &conn) { onConnectionError(conn); });
    conn->setRecvMsgCallback(
            [this](const spTcpConnection &conn, MsgBuffer &buffer, Date date) {
                onReceiveMessage(conn, buffer, date);
            });

    // 当 TcpConnection 生命周期结束时, 负责清理资源
    conn->setCleanupCallback([this](const spTcpConnection &conn) { removeConnection(conn); });

    {
        // 保存智能指针对象, 只要它还在连接列表里面, TcpConnection 对象就不会析构
        // 其它地方使用 const spTcpConnection& 即可, 无需拷贝智能指针对象, 节省开销
        std::lock_guard<std::mutex> lock(mtx_);
        connectionList_[clientFd] = conn;
        connectionNums_++;
    }

    // 调用回调函数, 该函数是虚函数, 可被子类重载
    onNewConnection(conn);

    // 把 TcpConnection 绑定的 Channel 添加到分配的 EventLoop
    loop->addChannel(conn->getChannel());
}

void TcpServer::removeConnection(const spTcpConnection &conn) {
    std::lock_guard<std::mutex> lock(mtx_);
    LOG_DEBUG("Cleanup TcpConnection: %s", conn->getPeerAddress().getIpPort().c_str());
    // 从连接列表删除智能指针, TcpConnection 对象析构
    // 自动从 Epoll 中删除 Channel, 关闭打开的 fd
    connectionList_.erase(conn->getFd());
    connectionNums_--;
}

void TcpServer::onConnectionError() {
    LOG_ERROR("Acceptor error: %s", strerror(errno));
}

void TcpServer::start() {
    eventLoopThreadPool_.start();
    mainLoop_->addChannel(&acceptorChannel_);  // acceptor 开始工作
    mainLoop_->loop();
}

void TcpServer::stop() {
    close(idleFd_);
    mainLoop_->quit();
    eventLoopThreadPool_.stop();
    onServerClose();
}
