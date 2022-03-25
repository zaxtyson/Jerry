//
// Created by zaxtyson on 2022/3/24.
//

#include "SslContext.h"
#include "logger/Logger.h"

namespace jerry::net {

#ifdef USE_OPENSSL

SslContext::SslContext() {
    // https://www.openssl.org/docs/man3.0/man3/SSL_CTX_new.html
    ctx = SSL_CTX_new(TLS_method());
}

SslContext::~SslContext() {
    SSL_CTX_free(ctx);
}

void SslContext::DisableOldVersion() {
    // https://www.openssl.org/docs/man3.0/man3/SSL_CTX_set_min_proto_version.html
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
}

void SslContext::EnableValidation() {
    SSL_CTX_set_default_verify_paths(ctx);
}

void SslContext::SetCertPath(std::string_view cert_path, std::string_view key_path) {
    char err_msg[4096]{};

    int rc = SSL_CTX_use_certificate_chain_file(ctx, cert_path.data());
    if (rc != 1) {
        ERR_error_string_n(ERR_get_error(), err_msg, sizeof(err_msg));
        LOG_FATAL("SSL load certificate chain file failed: %s, %s", cert_path.data(), err_msg)
    }

    rc = SSL_CTX_use_PrivateKey_file(ctx, key_path.data(), SSL_FILETYPE_PEM);
    if (rc != 1) {
        ERR_error_string_n(ERR_get_error(), err_msg, sizeof(err_msg));
        LOG_FATAL("SSL load private key file failed: %s, %s", cert_path.data(), err_msg)
    }

    rc = SSL_CTX_check_private_key(ctx);
    if (rc != 1) {
        ERR_error_string_n(ERR_get_error(), err_msg, sizeof(err_msg));
        LOG_FATAL("SSL private key not matches the certificate public key, %s", err_msg)
    }
}

SslCtx* SslContext::Get() const {
    return ctx;
}

#else  // no USE_OPENSSL
SslContext::SslContext() {}
SslContext::~SslContext() {}
void SslContext::DisableOldVersion() {}
void SslContext::EnableValidation() {}
void SslContext::SetCertPath(std::string_view cert_path, std::string_view key_path) {}
SslCtx* SslContext::Get() const {
    return nullptr;
}
#endif

}  // namespace jerry::net