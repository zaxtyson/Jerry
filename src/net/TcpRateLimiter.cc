//
// Created by zaxtyson on 2022/3/15.
//

#include "TcpRateLimiter.h"
#include "IOWorker.h"
#include "TcpConn.h"

namespace jerry::net {

TcpRateLimiter::TcpRateLimiter(const TcpRateLimiterConfig& config) : config{config} {}

bool TcpRateLimiter::ReachConnectionLimits(TcpConn* conn) {
    // TODO: Implement tcp rate limiter
    return conn->GetIOWorker()->GetConnNums() >= config.max_conns_per_io_worker;
}

bool TcpRateLimiter::ReachQpsLimits([[maybe_unused]] TcpConn* conn) {
    return false;
}

}  // namespace jerry::net