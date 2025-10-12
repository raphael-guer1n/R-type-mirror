# R-Type Multiplayer Protocol (RFC - Quake Style)

**Category:** Experimental  
**Author:** R-Type Team  
**Status:** Draft  
**Date:** 2025-09-30  

---

## 1. Introduction

This document specifies the network protocol used for the R-Type multiplayer project.  
It is based on **UDP** and a **server-authoritative architecture**.  
The protocol borrows concepts from the **Quake networking model**:

- Compact client commands (`UserCmd`) to minimize input bandwidth.  
- Server-to-client **snapshots** for efficient state sync.  
- Sequence numbers (`seq`) for ordering; no acknowledgment field in the header.

Goals:  
- Low latency suitable for action gameplay.  
- Bandwidth efficiency by compressing repeated state.  
- Graceful handling of out-of-order or lost packets. 

---

## 2. Conventions

- All integers are little-endian.  
- Floating point values use IEEE-754 binary32 (32-bit), little-endian.  
- All packets start with a **Packet Header**.  
- Time is expressed in **ticks**, controlled by the server.  
- UDP is the transport layer. No retransmission is guaranteed.  

---

## 3. Packet Header

Every packet begins with the following header:

| Field   | Type                              | Size | Description                                      |
|---------|-----------------------------------|------|--------------------------------------------------|
| `type`  | Unsigned integer (16-bit)         | 2    | Packet type (see Section 4)                      |
| `size`  | Unsigned integer (16-bit)         | 2    | Payload size in bytes (header excluded)          |
| `seq`   | Unsigned integer (32-bit)         | 4    | Sequence number of this packet (monotonic)       |

**Total size:** 8 bytes  

---

## 4. Packet Types

### 4.1 CONNECT_REQ (0x01)

Client → Server

| Field      | Type                             | Size | Description                  |
|------------|----------------------------------|------|------------------------------|
| `clientId` | Unsigned integer (32-bit)        | 4    | Client-provided identifier   |

---

### 4.2 CONNECT_ACK (0x02)

Server → Client

| Field            | Type                             | Size | Description                     |
|------------------|----------------------------------|------|---------------------------------|
| `serverId`       | Unsigned integer (32-bit)        | 4    | Unique server identifier        |
| `tickRate`       | Unsigned integer (32-bit)        | 4    | Ticks per second                |
| `playerEntityId` | Unsigned integer (32-bit)        | 4    | Entity ID controlled by client  |

---

### 4.3 INPUT (0x03)

Client → Server

| Field       | Type                                | Size | Description                                    |
|-------------|-------------------------------------|------|------------------------------------------------|
| `clientId`  | Unsigned integer (32-bit)           | 4    | ID of the controlled player/entity             |
| `tick`      | Unsigned integer (32-bit)           | 4    | Client-local tick when input was captured      |
| `keyCount`  | Unsigned integer (16-bit)           | 2    | Number of following key codes                  |
| `keys[i]`   | Signed integer (32-bit each)        | 4×N  | Sequence of 32-bit key codes (impl.-defined)   |

---

### 4.4 SNAPSHOT (0x04)

Server → Clients

| Field         | Type                             | Size | Description                        |
|---------------|----------------------------------|------|------------------------------------|
| `tick`        | Unsigned integer (32-bit)        | 4    | Tick number of snapshot            |
| `entityCount` | Unsigned integer (16-bit)        | 2    | Number of entities in snapshot     |

**EntityState** (repeated `entityCount` times)

| Field       | Type                                | Size | Description                         |
|-------------|-------------------------------------|------|-------------------------------------|
| `entityId`  | Unsigned integer (32-bit)           | 4    | Unique entity ID                    |
| `x`         | Float32 (IEEE-754)                  | 4    | X coordinate                        |
| `y`         | Float32 (IEEE-754)                  | 4    | Y coordinate                        |
| `vx`        | Float32 (IEEE-754)                  | 4    | Velocity X                          |
| `vy`        | Float32 (IEEE-754)                  | 4    | Velocity Y                          |
| `type`      | Unsigned integer (8-bit)            | 1    | Entity type code                    |
| `hp`        | Unsigned integer (8-bit)            | 1    | Health points                       |
| `collided`  | Unsigned integer (8-bit)            | 1    | Collision marker (0 or 1)           |

---

### 4.5 EVENT (0x05)

Server → Clients

| Field       | Type                             | Size | Description                     |
|-------------|----------------------------------|------|---------------------------------|
| `tick`      | Unsigned integer (32-bit)        | 4    | Tick number                     |
| `eventType` | Unsigned integer (16-bit)        | 2    | Event type (e.g., death, spawn) |
| `entityId`  | Unsigned integer (32-bit)        | 4    | Related entity ID                |

---

### 4.6 PING (0x06) / PONG (0x07)

| Field       | Type                             | Size | Description                               |
|-------------|----------------------------------|------|-------------------------------------------|
| `timestamp` | Unsigned integer (64-bit)        | 8    | Monotonic timestamp (implementation-defined) |

---

## 5. Message Flow

1. **Handshake**  
   - Client → CONNECT_REQ  
   - Server → CONNECT_ACK  

2. **Game Loop**  
   - Client → INPUT (keys pressed)  
   - Server → SNAPSHOT (world state)  
   - Server → EVENT (when needed)  

3. **Keep-Alive**  
   - Client → PING  
   - Server → PONG  

---

## 6. Error Handling

- Invalid packets MUST be discarded.  
- Timeout: clients inactive >5s SHOULD be dropped.  
- `seq` helps detect out-of-order packets; no `ack` field is present in the header.  

---

## 7. Security Considerations

- No authentication or encryption.  
- Protocol intended for trusted LAN or coursework environment.  
- DTLS/TLS MAY be considered for production.  