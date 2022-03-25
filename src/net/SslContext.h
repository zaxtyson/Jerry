//
// Created by zaxtyson on 2022/3/24.
//

#ifndef JERRY_SSLCONTEXT_H
#define JERRY_SSLCONTEXT_H

#ifdef USE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
using SslCtx = SSL_CTX;
#else
using SslCtx = void;
#endif

#include <map>
#include <string>

namespace jerry::net {

class SslContext {
  public:
    SslContext();
    ~SslContext();

    void DisableOldVersion();
    void EnableValidation();
    void SetCertPath(std::string_view cert_path, std::string_view key_path);
    SslCtx* Get() const;

  private:
    SslCtx* ctx{};
};

}  // namespace jerry::net


#endif  // JERRY_SSLCONTEXT_H
