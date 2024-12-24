#pragma once

#include <cstdint>

enum class Command : uint8_t{
    kDisplayOnlineUsers,
    kChatWithUser,
};

constexpr char kServerIp[] = "127.0.0.1";
constexpr uint16_t kServerPort = 8080;
