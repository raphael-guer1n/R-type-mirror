/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** packet
*/

#pragma once
#include <cstdint>

enum class PacketType : uint16_t {
    CONNECT_REQ = 1,
    CONNECT_ACK = 2,
    INPUT       = 3,
    SNAPSHOT    = 4,
    EVENT       = 5,
    PING        = 6,
    PONG        = 7
};

#pragma pack(push, 1)
struct PacketHeader {
    uint16_t type;
    uint16_t size;
    uint32_t seq;
};
#pragma pack(pop)

struct ConnectReq {
    uint32_t clientId;
};

struct ConnectAck {
    uint32_t serverId;
    uint32_t tickRate;
};

struct InputPacket {
    uint32_t clientId;
    uint32_t tick;
    uint8_t keys;
};
