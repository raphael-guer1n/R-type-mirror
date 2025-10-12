# Architecture Overview

This document describes the runtime architecture of the project in a language- and library-agnostic way. It reflects the current implementation: a server-authoritative model over UDP, a deterministic server loop at ~60 Hz, and an ECS-based game logic.

## High-level System View

```mermaid
flowchart LR
  %% --- SERVER SIDE ---
  subgraph Server
    Loop[Main Loop (~60 Hz)]
    NetS[UDP Socket (non-blocking)]
    ECS[ECS Systems]
  end

  %% --- CLIENT SIDE ---
  subgraph Client
    Input[Input Capture]
    NetC[UDP Socket]
    View[Presentation (ECS-backed)]
  end

  %% --- CONNECTIONS ---
  Input --> NetC
  NetC -->|INPUT| NetS
  NetS -->|SNAPSHOT / EVENT| NetC
  Loop --> ECS
  ECS --> Loop
  Loop -->|Broadcast SNAPSHOT| NetS
  NetC --> View
```

- Server: The authoritative source of truth. Applies inputs, advances the world each tick, spawns entities, resolves collisions/damage, and broadcasts state.
- Client: Captures user input, sends INPUT messages, consumes SNAPSHOT/EVENT updates, and renders. It runs lightweight local systems (control, integration, scrolling, animation) for presentation; server snapshots remain authoritative and overwrite state.

---

## Server Threading Model

```mermaid
flowchart TD
    MainLoop[Main Loop (~60 Hz)] --> ECS[ECS Systems]
    ECS -->|Update Entities| MainLoop
    MainLoop -->|non-blocking recv/send| Net[UDP Socket]
```

- Single deterministic main loop (~60 Hz).
- Networking is performed via a non-blocking UDP socket within the same thread.
- The loop never waits on clients; it continues to tick the simulation.

---

## Server Runtime Model

The server runs a deterministic main loop at ~60 Hz and uses a non-blocking UDP socket. The loop never waits on clients; it continues to tick the simulation.

Pseudocode overview:

```
initialize();
wait_for_players();
const tick_dt = 16 ms;
last_tick = now();

while (running) {
  // Networking (non-blocking)
  repeat {
    pkt = try_receive(); // returns none if no packet
    if (!pkt) break;
    process_network_input(pkt); // INPUT handling, validation
  }

  // Fixed-step simulation (~60 Hz)
  if (now() - last_tick >= tick_dt) {
    game_logic_tick();   // systems: movement, collisions, AI, damage, spawns
    broadcast_snapshot(); // cap entity count per packet for MTU safety
    tick++;
    last_tick += tick_dt;
    // catch-up: if drift accumulated, clamp to now()
    if (now() - last_tick >= tick_dt) last_tick = now();
  }
}
```

Notes:
- UDP I/O is non-blocking; dropped or late packets do not stall the simulation.
- World updates and networking happen in the same main loop for predictability.
- Snapshots currently cap the number of entities per packet to bound size.

---

## Client Runtime Model

Each frame the client:
1) Collects input (pressed/released keys).  
2) Sends a compact INPUT message (tick + variable-length key list).  
3) Consumes all available network packets (notably SNAPSHOT/EVENT).  
4) Runs local presentation systems (e.g., control to reset velocities, position integration with deltaTime, background scrolling for decor, animation updates).  
5) Applies the latest authoritative world from SNAPSHOT (overwriting as needed).  
6) Renders the frame.

This preserves responsiveness while remaining faithful to the server-authoritative model.

---

## Networking Model (summary)

- Transport: UDP, binary protocol with a fixed 8-byte header.  
- Direction: Client→Server INPUT at ~display rate; Server→Client SNAPSHOT/EVENT at tick rate.  
- Authority: Server dictates world state; clients do not simulate ownership.  
- Reliability: No per-packet ACK in the header; clients may retry handshakes.  
- See `docs/protocol/PROTOCOL.md` for exact field formats and sizes.

---

## Game Logic and ECS

The game logic follows an Entity-Component-System approach:

- Entity: Opaque identifier.  
- Component: Data containers (e.g., position, velocity, hitbox, health, collision state, kind, projectile tag, AI traits).  
- System: Stateless logic operating over component sets each tick (e.g., movement/integration, collisions/damage, AI behaviors, spawn/despawn, animation updates).

Representative server-side systems:
- Movement/integration (fixed timestep).  
- Projectile update/cleanup (advance projectiles, lifetime expiry).  
- Collision + damage with cooldown; sets collision_state which is propagated as a flag in snapshots.  
- AI behaviors (enemy patterns, boss phases) and spawn logic.  
- Snapshot building: collects a bounded set of active entities into a packet.

Representative client-side systems (presentation):
- control_system: resets velocities for controllable entities each frame (inputs define new velocity).  
- position_system: integrates positions using current velocities and deltaTime.  
- scroll_reset_system: maintains continuous background scrolling for decor entities.  
- animation_system + draw: updates sprite frames and renders by layer.

---

## Layers (agnostic)

1) Presentation layer — draws sprites/UI; no game rules.  
2) Networking layer — sends/receives binary UDP packets; parses/serializes messages.  
3) Game logic layer — ECS: maintains world state and runs systems at a fixed rate.  
4) Input layer (client) — captures user inputs and generates INPUT messages.

---

## Performance & Limits

- Target rate: ~60 ticks/s on the server.  
- Network safety: snapshot entity count capped (to fit typical MTU ~1500 bytes).  
- Non-blocking UDP ensures the main loop remains responsive under packet loss or delay.

---

## Fault Tolerance

- The server loop proceeds regardless of client delays/crashes.  
- Unknown or malformed packets are ignored per protocol spec.  
- Client state reconstruction relies on the latest received snapshot.