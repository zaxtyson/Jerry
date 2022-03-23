//
// Created by zaxtyson on 2022/3/13.
//

#ifndef JERRY_BASEBUFFER_H
#define JERRY_BASEBUFFER_H

#include <sys/types.h>  // ssize_t
#include <mutex>
#include <string_view>
#include <vector>

namespace jerry::net {

class BaseBuffer {
  public:
    using value_type = char;
    using iterator = std::vector<value_type>::iterator;
    using const_iterator = std::vector<value_type>::const_iterator;

  public:
    explicit BaseBuffer(size_t init_size);
    ~BaseBuffer() = default;

    ssize_t ReadBytesFromFd(int fd);
    ssize_t WriteBytesToFd(int fd);

    size_t ReadableBytes() const;
    size_t WritableBytes() const;

    void DropBytes(size_t n);
    void DropAllBytes();
    void DropBytesUntil(const_iterator pos);
    void Release();

    void Append(const value_type* data, size_t len);
    void Append(const_iterator begin, const_iterator end);
    void Append(std::basic_string_view<value_type> data);

    std::string_view ToStringView() const;
    std::string ToString() const;

  public:
    iterator BeginOfReadable();
    iterator BeginOfWritable();
    iterator EndOfReadable();
    iterator EndOfWritable();

    const_iterator BeginOfReadable() const;
    const_iterator BeginOfWritable() const;
    const_iterator EndOfReadable() const;
    const_iterator EndOfWritable() const;

  private:
    void EnsureWritableSpace(size_t len);

  private:
    //std::vector<value_type> header{};  // proto header if needed
    std::vector<value_type> buffer;    // payload data
    iterator read_idx{std::begin(buffer)};
    iterator write_idx{std::begin(buffer)};
};
}  // namespace jerry::net


#endif  // JERRY_BASEBUFFER_H
