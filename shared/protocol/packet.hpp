/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** packet
*/

#pragma once
#include <cstdint>

#pragma pack(push, 1)

namespace protocol {

    enum class PacketType : uint16_t {
        CONNECT_REQ = 1,
        CONNECT_ACK = 2,
        INPUT       = 3,
        SNAPSHOT    = 4,
        EVENT       = 5,
        PING        = 6,
        PONG        = 7
    };

    struct PacketHeader {
        uint16_t type;
        uint16_t size;
        uint32_t seq;
    };

    struct ConnectReq {
        uint32_t clientId;
    };

    struct ConnectAck {
        uint32_t playerEntityId;
        uint32_t tickRate;
    };

    struct InputPacket {
        uint32_t clientId;
        uint32_t tick;
        uint16_t keys;
    };

    struct EntityState {
        uint32_t entityId;
        float    x;
        float    y;
        float    vx;
        float    vy;
        uint8_t  type;
        uint8_t  hp;
    };

    struct Snapshot {
        uint32_t tick;
        uint16_t entityCount;
        EntityState entities[];
    };

    struct EventPacket {
        uint32_t tick;
        uint16_t eventType;
        uint32_t entityId;
    };

    struct PingPacket {
        uint64_t timestamp;
    };

    struct PongPacket {
        uint64_t timestamp;
    };

}

#pragma pack(pop)
