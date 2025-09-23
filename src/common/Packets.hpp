#pragma once
#include <cstdint>
#include <vector>
#include <cstring> // memcpy

#pragma pack(push, 1)

struct PacketHeader
{
    uint16_t type;
    uint16_t size; // payload size
    uint32_t seq;
};

enum PacketType : uint16_t
{
    CONNECT_REQ = 1,
    CONNECT_ACK = 2,
    INPUT = 3,
    SNAPSHOT = 4,
    EVENT_PKT = 5,
    PING = 6,
    PONG = 7
};

struct ConnectReq
{
    uint32_t clientId;
};
struct ConnectAck
{
    uint32_t serverId;
    uint32_t tickRate;
};
struct InputPacket
{
    uint32_t clientId;
    uint32_t tick;
    uint8_t keys;
};

struct EntityState
{
    uint32_t entityId;
    float x, y;
    float vx, vy;
    uint8_t type;
    uint8_t hp;
};

struct Snapshot
{
    uint32_t tick;
    uint16_t entityCount;
    // followed by `EntityState[entityCount]`
};

struct EventPacket
{
    uint32_t tick;
    uint16_t eventType;
    uint32_t entityId;
};
struct PingPacket
{
    uint64_t timestamp;
};

#pragma pack(pop)