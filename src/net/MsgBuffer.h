//
// Created by zaxtyson on 2021/9/21.
//

#ifndef JERRY_MSGBUFFER_H
#define JERRY_MSGBUFFER_H

#include <unistd.h>
#include <cstdint>
#include <utils/NonCopyable.h>
#include <vector>
#include <string>

// https://blog.csdn.net/Solstice/article/details/6329080

constexpr uint64_t kMsgBufferInitSize = 1024;
constexpr uint64_t kMsgBufferPrependSize = 8;

/**
 * 缓存类
 */
class MsgBuffer : NonCopyable {
public:
    explicit MsgBuffer(uint64_t initSize = kMsgBufferInitSize);

    ~MsgBuffer() = default;

    /**
     * 有多少字节数据可读
     * @return
     */
    size_t readableBytes() const { return writeIndex_ - readIndex_; }

    /**
     * 有多少字节空间可以写
     * @return
     */
    size_t writeableBytes() const { return buffer_.size() - writeIndex_; }

    /**
     * Buffer 头部预留空间大小
     * @return
     */
    size_t prependableBytes() const { return readIndex_; }


    /**
     * 内部容器的大小
     * @return
     */
    size_t size() const { return buffer_.size(); };

    /**
     * 内部容器的容量
     * @return
     */
    size_t capacity() const { return buffer_.capacity(); };

    /**
     * 移除 len 字节可读数据
     * 没有真的删除数据, 只是 readIndex 后移了
     * @param len
     */
    void drop(size_t len);

    /**
     * 移除在此之前的数据
     * @param end
     */
    void dropUntil(const char *end);

    /**
     * 移除所有可读数据
     * 其实是把各个 index 复位了
     */
    void dropAll();

    /**
     * 读走 len 字节数据, 作为字符串返回
     * @param len 数据长度
     * @return
     */
    std::string pop(size_t len);


    /**
     * 读走所有可读数据, 作为字符串返回
     * @return
     */
    std::string popAll();

    /**
     * 向 Buffer 添加数据
     * @param data 数据起始位置的指针
     * @param len 数据长度
     */
    void append(const char *data, size_t len);

    void append(const std::string &data);

    /**
     * 是否以某个字符串开头
     * @param str
     * @return
     */
    bool startsWith(const char *str);

    /**
     * 搜索子字符串
     * @param str
     * @return 字符串的位置, 如果没有就返回 readEnd() 所在位置
     */
    char *search(const char *str);

    /**
     * 从 fd 读取数据到 Buffer
     * @param fd socket fd
     * @param errnoBackup 读取过程中发生错误, errno 将提供此参数返回
     * @return
     */
    ssize_t readFromFd(int fd, int *errnoBackup);

public:
    /**
     * 可读数据的起始位置
     * @return
     */
    char *readBegin() { return begin() + readIndex_; }

    const char *readBegin() const { return begin() + readIndex_; }

    /**
     * 可写空间的起始位置
     * @return
     */
    char *writeBegin() { return begin() + writeIndex_; }

    const char *writeBegin() const { return begin() + writeIndex_; }

    /**
     * 可读数据的结束位置
     * @return
     */
    char *readEnd() { return writeBegin(); }

    const char *readEnd() const { return writeBegin(); }

private:
    char *begin() { return buffer_.data(); }

    const char *begin() const { return buffer_.data(); }

    /**
     * 确保 Buffer 能容纳下这么多数据, 不够就扩展空间
     * @param len
     */
    void ensureWriteableBytes(size_t len);

    /**
     * 扩展 vector 空间, 先尝试移动内部数据, 不够的话就扩容
     * @param len
     */
    void makeSpace(uint64_t len);

private:
    std::vector<char> buffer_;
    size_t readIndex_;
    size_t writeIndex_;
};


#endif //JERRY_MSGBUFFER_H
