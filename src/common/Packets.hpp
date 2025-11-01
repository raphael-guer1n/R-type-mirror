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
    CONNECT_ACK,
    INPUT_PKT,
    SNAPSHOT,
    EVENT_PKT,
    PING,
    PONG,
    GAME_OVER,
    LIST_LOBBIES,
    LOBBY_LIST_RESPONSE,
    CREATE_LOBBY,
    JOIN_LOBBY
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

/**
 * @brief Game over payload.
 */
struct GameOverPayload {
    uint32_t winnerEntityId;
};

/**
 * @brief Information about a single lobby.
 */
struct LobbyInfo
{
    uint8_t id;
    uint8_t playerCount;
    uint8_t maxPlayers;
    char name[32];
};

/**
 * @brief Response containing a list of lobbies.
 */
struct LobbyListResponse
{
    uint8_t count;
    LobbyInfo lobbies[4];
};

/**
 * @brief Request to create a new lobby
 */
struct LobbyCreateRequest {
    char name[32];
};

/**
 * @brief Request to join a lobby.
 */
struct LobbyJoinRequest
{
    uint8_t lobbyId;
};

#pragma pack(pop)