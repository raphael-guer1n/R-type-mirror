# R-Type Multiplayer Protocol (RFC)

**Category:** Experimental  
**Author:** R-Type Team  
**Status:** Draft  
**Date:** 2025-09-30  

---

## 1. Introduction

This document specifies the network protocol used for the R-Type multiplayer project.  
The protocol is based on **UDP** and is designed for a **server-authoritative architecture**.  
It defines the packet structure, message flow, and semantics required for clients and servers to interoperate.

The goals of this specification are:  
- Low latency for real-time gameplay  
- Minimal bandwidth usage  
- Deterministic synchronization between server and clients  
- Robustness against packet loss and reordering  

---

## 2. Conventions

The key words **MUST**, **MUST NOT**, **SHOULD**, **SHOULD NOT**, and **MAY** are to be interpreted as described in RFC 2119.  

- All integers are little-endian unless otherwise specified.  
- All packets begin with a **Packet Header**.  
- Time is expressed in **ticks**, synchronized by the server.  
- UDP is used as the transport layer; no retransmission is guaranteed.  

---

## 3. Packet Header

Every packet begins with the following header:

| Field       | Type     | Size | Description                           |
|-------------|----------|------|---------------------------------------|
| `type`      | uint16_t | 2    | Packet type (see Section 4)           |
| `size`      | uint16_t | 2    | Total packet size in bytes            |
| `seq`       | uint32_t | 4    | Sequence number (increment per send)  |

**Total size:** 8 bytes  

---

## 4. Packet Types

### 4.1 CONNECT_REQ (0x01)

Sent by the client to initiate a connection.

| Field       | Type     | Size | Description                           |
|-------------|----------|------|---------------------------------------|
| `clientId`  | uint32_t | 4    | Client-provided identifier            |

---

### 4.2 CONNECT_ACK (0x02)

Sent by the server in response to CONNECT_REQ.

| Field            | Type     | Size | Description                           |
|------------------|----------|------|---------------------------------------|
| `serverId`       | uint32_t | 4    | Unique server identifier              |
| `playerEntityId` | uint32_t | 4    | Entity ID controlled by this client   |
| `tickRate`       | uint32_t | 4    | Number of ticks per second            |

---

### 4.3 INPUT (0x03)

Sent by the client to indicate player input.

| Field      | Type     | Size | Description                           |
|------------|----------|------|---------------------------------------|
| `playerId` | uint32_t | 4    | Player entity ID                      |
| `tick`     | uint32_t | 4    | Tick at which input was sampled       |
| `inputs`   | uint8_t  | 1    | Bitmask of pressed keys               |

**Bitmask example:**  
- bit 0 = UP  
- bit 1 = DOWN  
- bit 2 = LEFT  
- bit 3 = RIGHT  
- bit 4 = SHOOT  

---

### 4.4 SNAPSHOT (0x04)

Sent by the server to broadcast the current world state.  

**SnapshotHeader**

| Field         | Type     | Size | Description                     |
|---------------|----------|------|---------------------------------|
| `tick`        | uint32_t | 4    | Tick of this snapshot           |
| `entityCount` | uint16_t | 2    | Number of entities serialized   |

**EntityState** (repeated `entityCount` times)

| Field      | Type     | Size | Description                       |
|------------|----------|------|-----------------------------------|
| `entityId` | uint32_t | 4    | Unique ID of the entity           |
| `x`        | float    | 4    | X coordinate                      |
| `y`        | float    | 4    | Y coordinate                      |
| `vx`       | float    | 4    | Velocity X                        |
| `vy`       | float    | 4    | Velocity Y                        |
| `type`     | uint8_t  | 1    | Entity type (player, enemy, etc.) |
| `hp`       | uint8_t  | 1    | Health points                     |

---

### 4.5 EVENT (0x05)

Sent by the server to notify critical events.

| Field     | Type     | Size | Description                          |
|-----------|----------|------|--------------------------------------|
| `eventId` | uint32_t | 4    | Event identifier                     |
| `data`    | uint32_t | 4    | Event-specific data                  |

Examples:  
- Player death  
- Enemy spawn  
- Explosion  

---

### 4.6 PING (0x06) / PONG (0x07)

Used to measure latency and maintain keep-alive.  

- **PING**: client → server  
- **PONG**: server → client  

| Field     | Type     | Size | Description                          |
|-----------|----------|------|--------------------------------------|
| `time`    | uint64_t | 8    | Client timestamp in microseconds     |

---

## 5. Message Flow

1. **Connection**  
   - Client → CONNECT_REQ  
   - Server → CONNECT_ACK  

2. **Game Loop**  
   - Client → INPUT (each tick)  
   - Server → SNAPSHOT (periodically, usually every tick or every few ticks)  
   - Server → EVENT (on critical events)  

3. **Keep-Alive**  
   - Client → PING  
   - Server → PONG  

---

## 6. Error Handling

- Packets with invalid size or unknown type MUST be discarded.  
- Clients not sending INPUT or PING for more than 5 seconds SHOULD be disconnected.  
- Sequence numbers MAY be used to detect and discard out-of-order packets.  

---

## 7. Security Considerations

- Protocol assumes a trusted LAN or controlled environment.  
- No authentication or encryption is included in this specification.  
- For production deployment, TLS or DTLS SHOULD be considered.  
