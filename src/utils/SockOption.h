//
// Created by zaxtyson on 2021/9/20.
//

#ifndef JERRY_SOCKOPTION_H
#define JERRY_SOCKOPTION_H


/**
 * 禁用 Nagle 算法, 允许小包发送
 * 实时性要求较高的场景可用启用
 */
void setTcpNoDelay(int fd);

/**
 *  允许 TIME_WAIT 状态的 Socket 被立刻重用, 而不是等待 2MSL
 */
void setReuseAddr(int fd);

/**
 * 允许多个 Socket 绑定相同的源地址和端口, 要求内核版本 >= 3.9
 * 为了防止端口劫持, 绑定同一个地址的 Sockets 必须拥有相同的 effective UID
 * 使用本选项可以实现由内核提供的 accept 负载均衡
 */
void setReusePort(int fd);

/**
 * 是否启用 TCP 心跳检测机制,
 * 默认发送 keepalive 报文时间时间间隔时 7200s, 两次重试报文的间隔 75s, 重试次数 9 次
 * 使用 sysctl -a | grep keepalive 查看内核参数
 * 注意 keepalive 只能检测连接是否存活，不能检测连接是否可用, 如应用层死锁
 * 仍然需要实现应用层的心跳包机制
 */
void setKeepAlive(int fd, int idleSecs = 7200, int intervalSecs = 75, int cnt = 9);

/**
 * 设置 Socket 为非阻塞模式
 */
void setNonBlockAndCloseOnExec(int fd);


#endif //JERRY_SOCKOPTION_H
