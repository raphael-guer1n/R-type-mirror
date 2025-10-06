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
- Server-to-client **snapshots** using delta compression for efficient state sync.  
- Reliability mechanisms via sequence numbers (`seq`) and acknowledgments (`ack`).

Goals:  
- Low latency suitable for action gameplay.  
- Bandwidth efficiency by compressing repeated state.  
- Graceful handling of out-of-order or lost packets. 

---

## 2. Conventions

- All integers are little-endian.  
- All packets start with a **Packet Header**.  
- Time is expressed in **ticks**, controlled by the server.  
- UDP is the transport layer. No retransmission is guaranteed.  

---

## 3. Packet Header

Every packet begins with the following header:

| Field   | Type     | Size | Description                                      |
|---------|----------|------|--------------------------------------------------|
| `type`  | uint16_t | 2    | Packet type (see Section 4)                      |
| `size`  | uint16_t | 2    | Total packet size in bytes                       |
| `seq`   | uint32_t | 4    | Sequence number of this packet (monotonic)       |
| `ack`   | uint32_t | 4    | Last sequence number received from peer          |

**Total size:** 12 bytes  

---

## 4. Packet Types

### 4.1 CONNECT_REQ (0x01)

Client → Server

| Field      | Type     | Size | Description                  |
|------------|----------|------|------------------------------|
| `clientId` | uint32_t | 4    | Client-provided identifier   |

---

### 4.2 CONNECT_ACK (0x02)

Server → Client

| Field            | Type     | Size | Description                     |
|------------------|----------|------|---------------------------------|
| `serverId`       | uint32_t | 4    | Unique server identifier        |
| `playerEntityId` | uint32_t | 4    | Entity ID controlled by client  |
| `tickRate`       | uint32_t | 4    | Ticks per second                |

---

### 4.3 INPUT (0x03)

Client → Server

**InputPacket**

| Field      | Type     | Size | Description                            |
|------------|----------|------|----------------------------------------|
| `clientId` | uint32_t | 4    | ID of client                          |
| `cmdCount` | uint8_t  | 1    | Number of commands included           |

**UserCmd** (repeated `cmdCount` times)

| Field   | Type    | Size | Description                             |
|---------|---------|------|-----------------------------------------|
| `tick`  | uint32  | 4    | Client tick for this command            |
| `dx`    | int16   | 2    | Movement on X (-1,0,+1)                 |
| `dy`    | int16   | 2    | Movement on Y (-1,0,+1)                 |
| `act`   | uint8   | 1    | Action bits (bit0=shoot, bit1=bomb…)    |

---

### 4.4 SNAPSHOT (0x04)

Server → Clients

**SnapshotHeader**

| Field         | Type     | Size | Description                        |
|---------------|----------|------|------------------------------------|
| `tick`        | uint32_t | 4    | Tick number of snapshot            |
| `entityCount` | uint16_t | 2    | Number of entities in snapshot     |

**EntityState** (repeated `entityCount` times)

| Field     | Type     | Size | Description                                |
|-----------|----------|------|--------------------------------------------|
| `entityId`| uint32_t | 4    | Unique entity ID                           |
| `flags`   | uint8_t  | 1    | Bitmask: which fields are present/changed  |
| `x`       | float    | 4    | X coordinate (if flagged)                  |
| `y`       | float    | 4    | Y coordinate (if flagged)                  |
| `vx`      | float    | 4    | Velocity X (if flagged)                    |
| `vy`      | float    | 4    | Velocity Y (if flagged)                    |
| `type`    | uint8_t  | 1    | Entity type (if flagged)                   |
| `hp`      | uint8_t  | 1    | Health points (if flagged)                 |

**Note:** Entities are delta-compressed: only fields with changes since last acknowledged snapshot are sent.  

---

### 4.5 EVENT (0x05)

Server → Clients

| Field     | Type     | Size | Description                     |
|-----------|----------|------|---------------------------------|
| `eventId` | uint32_t | 4    | Event identifier                |
| `data`    | uint32_t | 4    | Event-specific data             |

---

### 4.6 PING (0x06) / PONG (0x07)

| Field  | Type     | Size | Description                      |
|--------|----------|------|----------------------------------|
| `time` | uint64_t | 8    | Client timestamp in microseconds |

---

## 5. Message Flow

1. **Handshake**  
   - Client → CONNECT_REQ  
   - Server → CONNECT_ACK  

2. **Game Loop**  
   - Client → INPUT (with batched UserCmds)  
   - Server → SNAPSHOT (delta-compressed world state)  
   - Server → EVENT (critical game events)  

3. **Keep-Alive**  
   - Client → PING  
   - Server → PONG  

---

## 6. Error Handling

- Invalid packets MUST be discarded.  
- Timeout: clients inactive >5s SHOULD be dropped.  
- `seq`/`ack` numbers detect out-of-order and lost packets.  

---

## 7. Security Considerations

- No authentication or encryption.  
- Protocol intended for trusted LAN or coursework environment.  
- DTLS/TLS MAY be considered for production.  