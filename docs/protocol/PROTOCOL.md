# 📡 R-Type Network Protocol

## 1. Introduction

This document defines the network protocol used by **R-Type**.  

- **Transport** : UDP (User Datagram Protocol)  
- **Format** : binary (no JSON/text)
- **Authority** : the server is the single source of truth 
- **Client role** : send its inputs, display the received snapshots 

---

## 2. Common Header

All packets start with a fixed-size header of 8 bytes:

- Field: type
    - Size: 2 bytes (unsigned)
    - Meaning: Packet type identifier (see Section 3)
- Field: size
    - Size: 2 bytes (unsigned)
    - Meaning: Payload size in bytes (header excluded)
- Field: seq
    - Size: 4 bytes (unsigned)
    - Meaning: Monotonic sequence number emitted by the sender

Conventions:
- Integer endianness: little-endian
- Floating point: IEEE-754 binary32, little-endian
- Alignment/padding: Fields are tightly packed (no padding)

## 3. Packet Types

Type identifiers (enum):
- 1 = CONNECT_REQ
- 2 = CONNECT_ACK
- 3 = INPUT_PKT
- 4 = SNAPSHOT
- 5 = EVENT_PKT
- 6 = PING
- 7 = PONG
- 8 = GAME_OVER
- 9 = LIST_LOBBIES
- 10 = LOBBY_LIST_RESPONSE
- 11 = CREATE_LOBBY
- 12 = JOIN_LOBBY
- 13 = LOBBY_JOINED
- 14 = LEVEL_START
- 15 = LEVEL_END

Packet summary:

| Type | Name                   | Direction            | Payload overview                                      |
|------|------------------------|----------------------|-------------------------------------------------------|
| 1    | CONNECT_REQ            | Client → Server      | clientId                                              |
| 2    | CONNECT_ACK            | Server → Client      | serverId, tickRate, playerEntityId                    |
| 3    | INPUT                  | Client → Server      | clientId, tick, keyCount, keys[keyCount]              |
| 4    | SNAPSHOT               | Server → Client      | tick, entityCount, entities[entityCount]              |
| 5    | EVENT                  | Server → Client      | tick, eventType, entityId                             |
| 6/7  | PING/PONG              | Bidirectional        | timestamp                                             |
| 8    | GAME_OVER              | Server → Client      | winnerEntityId                                        |
| 9    | LIST_LOBBIES           | Client → Server      |                                                       |
| 10   | LOBBY_LIST_RESPONSE    | Server → Client      | count, lobbies[4]                                     |
| 11   | CREATE_LOBBY           | Client → Server      | name                                                  |
| 12   | JOIN_LOBBY             | Client → Server      | lobbyId                                               |
| 13   | LOBBY_JOINED           | Server → Client      | lobbyId                                               |
| 14   | LEVEL_START            | Server → Client      | level                                                 |
| 15   | LEVEL_END              | Server → Client      | level                                                 |

### 3.1 CONNECT_REQ (Client → Server)
Type = 1
Payload fields:
- clientId (4 bytes, unsigned): Temporary identifier chosen by the client

Purpose: Request a connection to the server.

### 3.2 CONNECT_ACK (Server → Client)
Type = 2
Payload fields:
- serverId (4 bytes, unsigned): Identifier assigned by the server
- tickRate (4 bytes, unsigned): Server frequency (ticks per second)
- playerEntityId (4 bytes, unsigned): Entity identifier controlled by this client

Purpose: Confirm connection and provide initial parameters.

### 3.3 INPUT (Client → Server)
Type = 3
Payload fields:
- clientId (4 bytes, unsigned): Identifier of the controlled player/entity
- tick (4 bytes, unsigned): Client-local tick when input was captured
- keyCount (2 bytes, unsigned): Number of following key codes
- keys (keyCount × 4 bytes, signed): Sequence of 32-bit key codes

Key code encoding:
- Each key is encoded as a 32-bit signed integer. The specific key mapping is implementation-defined (e.g., platform keycodes). Letters typically use their ASCII values; arrows and special keys use platform-specific constants.
- Servers should tolerate unknown codes and ignore duplicates within a single packet.


### 3.4 SNAPSHOT (Server → Client)
Type = 4
Payload fields:
- tick (4 bytes, unsigned)
- entityCount (2 bytes, unsigned)
- entities: repeated structure `EntityState` (entityCount times), each containing:
    - entityId (4 bytes, unsigned)
    - x (4 bytes, float32), y (4 bytes, float32)
    - vx (4 bytes, float32), vy (4 bytes, float32)
    - type (1 byte, enum; e.g., player, enemy, projectile)
    - hp (1 byte, unsigned)
    - collided (1 byte, 0 or 1)

Purpose: Send the world state for this tick.

Server note: `entityCount` is currently capped at 80 entities per snapshot to bound packet size.

Entity type codes (for `EntityState.type`):

| Code | Meaning     |
|------|-------------|
| 0    | unknown     |
| 1    | player      |
| 2    | enemy       |
| 3    | projectile  |
| 4    | pickup      |
| 5    | decor       |

### 3.5 EVENT (Server → Client)
Type = 5
Payload fields:
- tick (4 bytes, unsigned)
- eventType (2 bytes, unsigned): e.g., 1=PlayerDeath, 2=Spawn, 3=PowerUp
- entityId (4 bytes, unsigned)

Purpose: Signal a punctual event.

### 3.6 PING / PONG
Type = 6 / 7
Payload fields:
- timestamp (8 bytes, unsigned): Monotonic timestamp (format implementation-defined)

Purpose: Measure latency and keep the connection alive.

### 3.7 LIST_LOBBIES (Client → Server)
Type = 9
Payload: none
Purpose: Requests the server to send the current list of active lobbies.

### 3.8 LOBBY_LIST_RESPONSE (Server → Client)
Type = 10
Payload fields:
- count (1 byte, unsigned): Number of lobbies returned (max 4)
- lobbies[4]:
  - id (1 byte, unsigned)
  - playerCount (1 byte, unsigned)
  - maxPlayers (1 byte, unsigned)
  - name[32] (char array, zero-terminated)

Purpose: Responds to a LIST_LOBBIES request with information about existing lobbies.

#### Example Payload (2 lobbies):
- count = 0x02
- lobbies[0]:
    - id = 0x01
    - playerCount = 0x01
    - maxPlayers = 0x02
    - name = "AlphaLobby" + '\0' padding
- lobbies[1]:
    - id = 0x02
    - playerCount = 0x02
    - maxPlayers = 0x02
    - name = "BetaLobby" + '\0' padding

### 3.9 CREATE_LOBBY (Client → Server)
Type = 11
Payload fields:
- name[32] (char array, zero-terminated)

Purpose: Requests the server to create a new lobby. Name length is limited to 31 characters + null terminator.

#### Example Payload:
- name = "GammaLobby" + '\0' padding to 32 bytes

### 3.10 JOIN_LOBBY (Client → Server)
Type = 12
Payload fields:
- lobbyId (1 byte, unsigned)

Purpose: Requests to join an existing lobby.

#### Example Payload:
- lobbyId = 0x02

### 3.11 LOBBY_JOINED (Server → Client)
Type = 13
Payload fields:
- lobbyId (1 byte, unsigned)

Purpose: Confirms the client has successfully joined the specified lobby.

#### Example Payload:
- lobbyId = 0x02

### 3.12 LEVEL_START (Server → Client)
Type = 14
Payload fields:
- level (4 bytes, unsigned)

Purpose: Signals the start of a level. The client should transition from LOADING to PLAYING, trigger fade-in effects, and start HUD animations.

#### Example Payload:
- level = 0x00000001 (Level 1)

### 3.13 LEVEL_END (Server → Client)
Type = 15
Payload fields:
- level (4 bytes, unsigned)

Purpose: Signals the end of a level. The client should transition back to LOADING, reset fade effects, and prepare the next level.

#### Example Payload:
- level = 0x00000001 (Level 1 ended)

## 4. Binary Example
Example of an INPUT packet (2 keys pressed: 'q' and 'z'):
- Header:
    - `type` = 0x0003 (INPUT)
    - `size` = 18 bytes (0x0012)  // 4 (clientId) + 4 (tick) + 2 (keyCount) + 2×4 (keys)
    - `seq`  = 0x00000005

- Payload (little-endian):
    - `clientId` = 0x00000001
    - `tick`     = 0x0000003C (60)
    - `keyCount` = 0x0002
    - keys[0] = 0x00000071 ('q')
    - keys[1] = 0x0000007A ('z')

Note: Arrows and special keys use implementation-defined 32-bit codes, which may be larger than ASCII.

## 5. Reliability

- Protocol relies on UDP (no delivery guarantee).
- Packets carry a `seq` number for ordering/telemetry; there is currently no `ack` field in the header.
- Critical handshake (CONNECT_REQ/ACK): the client should retry `CONNECT_REQ` if no `CONNECT_ACK` is received after a timeout (implementation-dependent).
- Inputs and Snapshots are sent frequently; occasional loss is tolerated.

## 6. General Rules

- The server is authoritative: only its state is valid.  
- The client must:  
  - send INPUT regularly (~60 Hz),  
  - process SNAPSHOTs and EVENTs,  
  - interpolate/predict for smoothness.  
- Unknown data MUST be ignored for robustness.

## 7. Diagram

   ┌─────────────┐                         ┌─────────────┐
   │   CLIENT    │                         │   SERVER    │
   │ (Graphic)   │                         │ (Authority) │
   └──────┬──────┘                         └──────┬──────┘
          │                                     │
          │ 1. CONNECT_REQ                      │
          ├────────────────────────────────────>│
          │                                     │
          │ 2. CONNECT_ACK                      │
          │<────────────────────────────────────┤
          │                                     │
          │ 3. INPUT (keys)                     │
          ├────────────────────────────────────>│
          │                                     │
          │ 4. SNAPSHOT (world state)           │
          │<────────────────────────────────────┤
          │                                     │
          │ 5. EVENT (e.g. death, spawn)        │
          │<────────────────────────────────────┤
          │                                     │
          │ 6. PING → PONG (keep-alive, RTT)    │
          ├────────────────────────────────────>│
          │<────────────────────────────────────┤