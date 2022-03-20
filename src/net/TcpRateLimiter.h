//
// Created by zaxtyson on 2022/3/15.
//

#ifndef JERRY_TCPLIMITER_H
#define JERRY_TCPLIMITER_H

#include <cstddef>  // size_t
#include "utils/NonCopyable.h"

namespace jerry::net {

class TcpConn;

struct TcpRateLimiterConfig {
    size_t max_conns_per_ip = 0;
    size_t max_conns_per_io_worker = 0;
    size_t max_qps_per_host = 0;
    size_t max_qps_per_io_worker = 0;
};

class TcpRateLimiter : NonCopyable {
  public:
    explicit TcpRateLimiter(const TcpRateLimiterConfig& config);
    virtual ~TcpRateLimiter() = default;

  public:
    virtual bool ReachConnectionLimits(TcpConn* conn);
    virtual bool ReachQpsLimits(TcpConn* conn);

  private:
    TcpRateLimiterConfig config;
};
}  // namespace jerry::net


#endif  // JERRY_TCPLIMITER_H
