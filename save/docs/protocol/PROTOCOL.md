# 📡 R-Type Network Protocol

## 1. Introduction

This document defines the network protocol used by **R-Type**.  

- **Transport** : UDP (User Datagram Protocol)  
- **Format** : binary (no JSON/text)
- **Authority** : the server is the single source of truth 
- **Client role** : send its inputs, display the received snapshots 

---

## 2. Common Header

All packets start with an **8-byte** header:

```
#pragma pack(push, 1)
struct PacketHeader {
    uint16_t type;   -> packet identifier (enum)
    uint16_t size;   -> payload size in bytes
    uint32_t seq;    -> sequence number
};
#pragma pack(pop)
```

- type: packet identifier (enum)
- size: payload size in bytes
- seq: sequence number, incremented by the sender

## 3. Packet Types

### 3.1 CONNECT_REQ (Client → Server)
Type = 1
Payload :

```
struct ConnectReq {
    uint32_t clientId;  -> temporary identifier chosen by the client
};
```

Purpose: request a connection to the server.

### 3.2 CONNECT_ACK (Server → Client)
Type = 2
Payload :
```
struct ConnectAck {
    uint32_t serverId;   -> identifier assigned by the server
    uint32_t tickRate;   -> server frequency (Hz, e.g. 60)
};
```

Purpose: confirm connection and provide initial parameters.

### 3.3 INPUT (Client → Server)
Type = 3
Payload :
```
struct InputPacket {
    uint32_t clientId;  -> player id
    uint32_t tick;      -> local tick when input was sent
    uint8_t  keys;      -> key bitmap
};
```

Key mapping (example):
bit 0 = UP
bit 1 = DOWN
bit 2 = LEFT
bit 3 = RIGHT
bit 4 = SHOOT

### 3.4 SNAPSHOT (Server → Client)
Type = 4
Payload :
```
struct EntityState {
    uint32_t entityId;
    float    x, y;
    float    vx, vy;
    uint8_t  type;    -> player, enemy, projectile…
    uint8_t  hp;
};

struct Snapshot {
    uint32_t tick;          -> server tick
    uint16_t entityCount;   -> number of entities
    EntityState entities[]; ->  variable-sized array
};
```

Purpose: send the world state for this tick.

### 3.5 EVENT (Server → Client)
Type = 5
Generic payload:
```
struct EventPacket {
    uint32_t tick;
    uint16_t eventType;   -> ex: 1=PlayerDeath, 2=Spawn, 3=PowerUp
    uint32_t entityId;
};
```

Purpose: signal a punctual event.

### 3.6 PING / PONG
Type = 6 / 7
Payload :
```
struct PingPacket {
    uint64_t timestamp;
};
```

Purpose: measure latency and keep the connection alive.

## 4. Binary Example
Example of an INPUT packet:
- Header :
    - type = 0x0003 (INPUT)
    - size = 0x0009 (9 bytes payload)
    - seq = 0x00000005

- Payload :
    - clientId = 0x00000001
    - tick = 0x0000003C (decimal 60)
    - keys = 0b00010000 (tir)
```
Hex representation :
03 00 09 00 05 00 00 00  01 00 00 00  3C 00 00 00  10
```

## 5. Reliability

- The protocol relies on UDP (no delivery guarantee).
- Critical packets (connection) must be retransmitted if no response (timeout).
- Other packets (inputs, snapshots) are sent frequently → loss is tolerable.

## 6. General Rules

- The server is authoritative: only its state is valid.
- The client must:
    - send inputs regularly (~60 Hz),
    - display received snapshots,
    - ensure smoothness with interpolation/prediction.
- Any unknown data must be ignored (robustness).

## 7.  Diagram

   ┌─────────────┐                         ┌─────────────┐
   │   CLIENT    │                         │   SERVER    │
   │ (Graphic)   │                         │ (Authority) │
   └──────┬──────┘                         └──────┬──────┘
          │                                     │
          │ 1. CONNECT_REQ (UDP)                │
          ├────────────────────────────────────>│
          │                                     │
          │ 2. CONNECT_ACK (UDP)                │
          │<────────────────────────────────────┤
          │                                     │
          │ 3. INPUT (UDP, 60 Hz)               │
          ├────────────────────────────────────>│
          │                                     │
          │ 4. SNAPSHOT (UDP, 20–60 Hz)         │
          │<────────────────────────────────────┤
          │                                     │
          │ 5. EVENT (ex: mort, spawn boss)     │
          │<────────────────────────────────────┤
          │                                     │
          │ 6. PING → PONG (keep-alive, RTT)    │
          ├────────────────────────────────────>│
          │<────────────────────────────────────┤