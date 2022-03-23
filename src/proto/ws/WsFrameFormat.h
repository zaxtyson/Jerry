//
// Created by zaxtyson on 2022/3/23.
//

#ifndef JERRY_WSFRAMEFORMAT_H
#define JERRY_WSFRAMEFORMAT_H

#include <cstdint>

namespace jerry::proto::ws {

/*
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-------+-+-------------+-------------------------------+
    |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
    |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
    |N|V|V|V|       |S|             |   (if payload len==126/127)   |
    | |1|2|3|       |K|             |                               |
    +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
    |     Extended payload length continued, if payload len == 127  |
    + - - - - - - - - - - - - - - - +-------------------------------+
    |                               |Masking-key, if MASK set to 1  |
    +-------------------------------+-------------------------------+
    | Masking-key (continued)       |          Payload Data         |
    +-------------------------------- - - - - - - - - - - - - - - - +
    :                     Payload Data continued ...                :
    + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
    |                     Payload Data continued ...                |
    +---------------------------------------------------------------+
 */

// we MUST use `__attribute__((packed))` to disable data alignment
// to avoid that compiler inserting meaningless padding in our struct
struct ws_base_head {
    uint8_t opcode : 4, rsv3 : 1, rsv2 : 1, rsv1 : 1, fin : 1;
    uint8_t payload_length : 7, mask : 1;
    uint32_t mask_key;  // only when payload_length < 126 and mask = 1
} __attribute__((packed));

struct ws_ext_head_126 {
    uint16_t payload_length;
    uint32_t mask_key;
} __attribute__((packed));

struct ws_ext_head_127 {
    uint64_t payload_length;
    uint32_t mask_key;
} __attribute__((packed));

enum class WsFrameType : uint8_t {
    kContinue = 0x00,
    kText = 0x01,
    kBinary = 0x02,
    // 0x03~0x07 are reserved
    kClose = 0x08,
    kPing = 0x09,
    kPong = 0x0A
    // 0x0B~0x0F are reserved
};

}  // namespace jerry::proto::ws

#endif  // JERRY_WSFRAMEFORMAT_H
