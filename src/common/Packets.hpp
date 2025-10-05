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
    uint16_t type;
    uint16_t size; // payload size
    uint32_t seq;
    uint32_t ack;
};
/**    * @brief Different packet types for client-server communication.
    */  
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
    uint32_t playerEntityId;
};
/**    * @brief Input packet structure.
    */  

struct UserCmd
{
    uint32_t tick;   // tick du client
    int16_t dx;      // déplacement X (ex: -1 gauche, +1 droite, 0 rien)
    int16_t dy;      // déplacement Y (ex: -1 bas, +1 haut, 0 rien)
    uint8_t actions; // bits: 1=shoot, 2=bomb, etc.
};

/**    * @brief Input packet structure.
    */  
struct InputPacket
{
    uint32_t clientId;
    uint8_t cmdCount; // number of commands
    // followed by `UserCmd[cmdCount]`
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
    uint8_t flags; // bits for various states (e.g., alive, shooting)
};
/**    * @brief Snapshot packet structure.
    */  
struct SnapshotHeader
{
    uint32_t tick;
    uint16_t entityCount;
    uint8_t isDelta;    // 0 = full, 1 = delta
    uint32_t baseTick; // for delta compression
    // followed by `EntityState[entityCount]`
};
/**    * @brief Event packet structure.
    */  
struct EventPacket
{
    uint32_t tick;
    uint16_t eventType;
    uint32_t entityId;
};
/**    * @brief Ping packet structure.
    */  
struct PingPacket
{
    uint64_t timestamp;
};

#pragma pack(pop)