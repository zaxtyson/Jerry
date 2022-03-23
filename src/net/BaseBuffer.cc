//
// Created by zaxtyson on 2022/3/13.
//

#include "BaseBuffer.h"
#include <sys/uio.h>     // iovec
#include <sys/unistd.h>  // write

namespace jerry::net {

BaseBuffer::BaseBuffer(size_t init_size)
    : buffer(init_size), read_idx{std::begin(buffer)}, write_idx{std::begin(buffer)} {}

BaseBuffer::iterator BaseBuffer::BeginOfReadable() {
    return read_idx;
}

BaseBuffer::iterator BaseBuffer::BeginOfWritable() {
    return write_idx;
}

BaseBuffer::iterator BaseBuffer::EndOfReadable() {
    return write_idx;
}

BaseBuffer::const_iterator BaseBuffer::BeginOfReadable() const {
    return read_idx;
}

BaseBuffer::const_iterator BaseBuffer::BeginOfWritable() const {
    return write_idx;
}

BaseBuffer::const_iterator BaseBuffer::EndOfReadable() const {
    return write_idx;
}

BaseBuffer::iterator BaseBuffer::EndOfWritable() {
    return std::end(buffer);
}

BaseBuffer::const_iterator BaseBuffer::EndOfWritable() const {
    return std::cend(buffer);
}

size_t BaseBuffer::ReadableBytes() const {
    return std::distance(BeginOfReadable(), EndOfReadable());
}

size_t BaseBuffer::WritableBytes() const {
    return std::distance(BeginOfWritable(), EndOfWritable());
}

void BaseBuffer::DropBytes(size_t n) {
    if (n >= ReadableBytes()) {
        DropAllBytes();
    } else {
        read_idx += n;
    }
}

void BaseBuffer::DropAllBytes() {
    read_idx = std::begin(buffer);
    write_idx = std::begin(buffer);
}

void BaseBuffer::DropBytesUntil(BaseBuffer::const_iterator pos) {
    while (read_idx != pos) {
        read_idx += 1;
    }
}

void BaseBuffer::Release() {
    buffer = std::vector<char>();  // release memory
    read_idx = std::begin(buffer);
    write_idx = std::begin(buffer);
}

void BaseBuffer::EnsureWritableSpace(size_t len) {
    if (WritableBytes() >= len) {
        return;
    }

    // [ dropped |    data    |   unused   ]
    //           ↑            ↑
    //           read_idx     write_idx

    auto free_space = std::distance(std::begin(buffer), BeginOfReadable());

    if (free_space + WritableBytes() >= len) {
        std::move(BeginOfReadable(), EndOfReadable(), std::begin(buffer));
        read_idx = std::begin(buffer);
        write_idx = write_idx - free_space;
        return;
    }

    // not enough space, do realloc
    auto write_idx_back = std::distance(std::begin(buffer), write_idx);
    size_t new_size = std::max(buffer.size() + len, static_cast<size_t>(buffer.size() * 1.5));
    buffer.resize(new_size);
    read_idx = std::begin(buffer) + free_space;
    write_idx = std::begin(buffer) + write_idx_back;
}

void BaseBuffer::Append(const BaseBuffer::value_type* data, size_t len) {
    if (len == 0) {
        return;
    }
    EnsureWritableSpace(len);
    std::copy(data, data + len, BeginOfWritable());
    write_idx += len;
}

void BaseBuffer::Append(std::basic_string_view<BaseBuffer::value_type> data) {
    Append(data.data(), data.size());
}

void BaseBuffer::Append(BaseBuffer::const_iterator begin, BaseBuffer::const_iterator end) {
    auto len = std::distance(begin, end);
    Append(begin.base(), len);
}

std::string_view BaseBuffer::ToStringView() const {
    return {BeginOfReadable().base(), ReadableBytes()};
}

std::string BaseBuffer::ToString() const {
    return {BeginOfReadable(), EndOfReadable()};
}

ssize_t BaseBuffer::ReadBytesFromFd(int fd) {
    char extra_buffer[65535]{};
    size_t writable_bytes = WritableBytes();  // cache

    iovec vec[2];
    vec[0].iov_base = BeginOfWritable().base();
    vec[0].iov_len = writable_bytes;
    vec[1].iov_base = extra_buffer;
    vec[1].iov_len = sizeof(extra_buffer);

    // the remains of writable space may not enough, use extra_buffer to avoid
    // doing twice `read` system call
    int iov_cnt = writable_bytes < sizeof(extra_buffer) ? 2 : 1;
    ssize_t n = readv(fd, vec, iov_cnt);
    if (n <= 0) {  // error or peer closed
        return n;
    }

    if (n <= static_cast<ssize_t>(writable_bytes)) {
        write_idx += n;
    } else {
        // buffer is full, some data stored in extra_buffer
        write_idx = std::end(buffer);
        Append(extra_buffer, n - writable_bytes);
    }
    return n;
}

ssize_t BaseBuffer::WriteBytesToFd(int fd) {
    return write(fd, BeginOfReadable().base(), ReadableBytes());
}

}  // namespace jerry::net
