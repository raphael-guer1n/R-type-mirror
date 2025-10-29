#pragma once
#include <cstdint>
#include <vector>
#include <cstring> // memcpy

#pragma pack(push, 1)
/**
    * @file Packets.hpp
    * @brief Network packet structures and definitions for client-server communication.
    */  
struct PacketHeader
{
    uint8_t type;
    uint16_t size; // payload size
    uint32_t seq;
};
/**    * @brief Different packet types for client-server communication.
    */  
enum PacketType : uint16_t
{
    CONNECT_REQ = 1,
    CONNECT_ACK = 2,
    INPUT_PKT = 3,
    SNAPSHOT = 4,
    EVENT_PKT = 5,
    PING = 6,
    PONG = 7,
    GAME_OVER = 8
};
/**    * @brief Connect request packet structure.
    */  
struct ConnectReq
{
    uint32_t clientId;
};
/**    * @brief Connect acknowledgment packet structure.
    */
struct ConnectAck
{
    uint32_t serverId;
    uint32_t tickRate;
    uint16_t playerEntityId;
};
/**    * @brief Input packet structure.
    */  
struct InputPacket
{
    uint32_t clientId;
    uint32_t tick;
    uint16_t keyCount;
};
/**    * @brief State of a single entity in a snapshot.
    */  
struct EntityState
{
    uint32_t entityId;
    float x, y;
    float vx, vy;
    uint8_t type;
    uint8_t hp;
    bool collided;

    uint8_t platformType; // optional: used by platforms to indicate variant (0=regular)

    float hb_w;
    float hb_h;
    float hb_ox;
    float hb_oy;
};
/**    * @brief Snapshot packet structure.
    */  
struct Snapshot
{
    uint32_t tick;
    uint16_t entityCount;
    // followed by `EntityState[entityCount]`
};
/**    * @brief Event packet structure.
    */  
struct EventPacket
{
    uint32_t tick;
    uint8_t eventType;
    uint16_t entityId;
};
/**    * @brief Ping packet structure.
    */  
struct PingPacket
{
    uint64_t timestamp;
};

struct GameOverPayload {
    uint32_t winnerEntityId;
};

#pragma pack(pop)