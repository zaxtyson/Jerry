//
// Created by zaxtyson on 2021/9/30.
//

#ifndef JERRY_PRINTWRITER_H
#define JERRY_PRINTWRITER_H

#include <string>

class HttpResponse;

class PrintWriter {
public:
    friend class HttpResponse;

    PrintWriter() = default;

    ~PrintWriter() = default;

    template<typename T>
    void print(const T& data) {
        data_.append(data);
    }

private:

    const std::string &getData() const { return data_; }

private:
    std::string data_;
};


#endif //JERRY_PRINTWRITER_H
