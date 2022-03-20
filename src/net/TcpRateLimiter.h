//
// Created by zaxtyson on 2022/3/15.
//

#ifndef JERRY_TCPLIMITER_H
#define JERRY_TCPLIMITER_H
#include <utils/NonCopyable.h>

namespace jerry::net{
class TcpLimiter:NonCopyable {
  public:
    TcpLimiter() = default;
    virtual ~TcpLimiter() = default;

  public:
    virtual bool  
};
}



#endif  // JERRY_TCPLIMITER_H
