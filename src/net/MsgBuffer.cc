//
// Created by zaxtyson on 2021/9/21.
//

#include <net/MsgBuffer.h>
#include <sys/uio.h>
#include <cassert>
#include <cstring>
#include <algorithm>

MsgBuffer::MsgBuffer(uint64_t initSize) :
        buffer_(kMsgBufferPrependSize + initSize),
        readIndex_(kMsgBufferPrependSize),
        writeIndex_(kMsgBufferPrependSize) {

}

void MsgBuffer::drop(size_t len) {
    if (len >= readableBytes()) {
        dropAll();  // 标注为全部读取
    } else {
        readIndex_ += len;  // 可读下标后移
    }
}

std::string MsgBuffer::pop(size_t len) {
    len = len >= readableBytes() ? readableBytes() : len;
    std::string ret(readBegin(), len);
    drop(len);
    return ret;
}

void MsgBuffer::dropAll() {
    // 直接复位
    readIndex_ = writeIndex_ = kMsgBufferPrependSize;
}

std::string MsgBuffer::popAll() {
    return pop(readableBytes());
}

void MsgBuffer::ensureWriteableBytes(size_t len) {
    if (writeableBytes() < len) {
        makeSpace(len);
    }
}

void MsgBuffer::append(const char *data, size_t len) {
    ensureWriteableBytes(len);
    std::copy(data, data + len, writeBegin());
    writeIndex_ += len;
}

void MsgBuffer::append(const std::string &data) {
    append(data.data(), data.size());
}

void MsgBuffer::makeSpace(uint64_t len) {
    // 之前 drop, 导致 buffer 全面还有空间, 如果够用的话, 就把后面的数据移到前面去
    // 留足 len 字节 writeable 空间
    // 实在凑不起了就扩展 buffer
    if (prependableBytes() - kMsgBufferPrependSize + writeableBytes() >= len) {
        std::copy(readBegin(), writeBegin(), begin() + kMsgBufferPrependSize);
    } else {
        size_t readableBytes_ = readableBytes();  // 记录下有多少数据先
        buffer_.resize(writeIndex_ + len);
        readIndex_ = kMsgBufferPrependSize;  // readIndex_ 回到最开始的位置
        writeIndex_ = kMsgBufferPrependSize + readableBytes_;
    }
}

ssize_t MsgBuffer::readFromFd(int fd, int *errnoBackup) {
    // 这个实现确实妙啊, 一开始 buffer 不分配太多多空间, 避免浪费资源
    // 通过向量 IO 往栈上写数据, 减少系统调用次数
    // 如果数据过多, 读完再 append 到 vector, 实现自适应扩容

    char extraBuffer[65535] = {0};
    iovec vec[2];
    size_t writeableBytes_ = writeableBytes(); // 没写数据前, 剩下这么多空间

    vec[0].iov_base = writeBegin();
    vec[0].iov_len = writeableBytes_;
    vec[1].iov_base = extraBuffer;
    vec[1].iov_len = sizeof(extraBuffer);

    // 如果 vector 还没有扩容过(1024), 就启用栈上空间容纳可能的多余数据
    int iovcnt = writeableBytes_ < sizeof(extraBuffer) ? 2 : 1;
    ssize_t n = readv(fd, vec, iovcnt);
    if (n < 0) {
        *errnoBackup = errno; // 备份 errno
    } else if (n <= writeableBytes_) {
        writeIndex_ += n; // 数据不多, 没用到 extraBuffer
    } else {
        // 数据还有一部分在 extraBuffer, 我们把它追加到 vector
        writeIndex_ = buffer_.size();  // vector 已经满了
        append(extraBuffer, n - writeableBytes_);  // 溢出来的数据追加回去, vector 自动扩容 2 倍
    }
    return n;
}

void MsgBuffer::dropUntil(const char *end) {
    assert(end >= readBegin());
    assert(end <= readEnd());
    drop(end - readBegin());
}

bool MsgBuffer::startsWith(const char *str) {
    return strncmp(str, readBegin(), strlen(str)) == 0;
}

char *MsgBuffer::search(const char *str) {
    return std::search(readBegin(), readEnd(), str, str + strlen(str));
}




