#pragma once
#include <cstdint>

using length_t = uint32_t;
using user_id_t = uint32_t; // type to hold user IDs
enum class PacketType : uint8_t {
    kRequestRegister = 0,
    kRequestUsers = 1,
    kRequestSend = 2,
    kRequestRecv = 3,
    kRespRegister = 0xF0,
    kRespUsers = 0xF1,
    kRespSend = 0xF2,
    kRespRecv = 0xF3
};

enum class SendStatus : uint8_t {
    kSuccess = 0,
    kErrSrcId = 1,
    kErrTrgtId = 2,
};

struct __attribute__((packed)) Header {
    PacketType type;
    length_t len;
};

constexpr length_t kReqUsersPayloadLen = sizeof(user_id_t);
constexpr length_t kReqSendMinPayloadLen = 8;
constexpr length_t kReqRecvPayloadLen = sizeof(user_id_t);