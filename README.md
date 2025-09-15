# R-Type: A Game Engine that Roars!

This is our implementation of the **R-Type multiplayer game** for the B-CPP-500 project at **Epitech**.

The goal is twofold:
1. Implement a **network multiplayer version** of the legendary 90's Shoot'em Up, R-Type.
2. Build a **custom game engine** that demonstrates real-world architecture patterns, separation of concerns, and networking.

---

## Build and Run

### Requirements
- C++20 compatible compiler (GCC, Clang, or MSVC on Windows)
- [CMake](https://cmake.org/) (>= 3.18 recommended)
- Package manager: Conan, Vcpkg, or CMake-CPM

### Build instructions
```bash
git clone https://github.com/<your-repo>/r-type.git
cd r-type
mkdir build && cd build
cmake ..
cmake --build .
```

Binaries:
- `r-type_server`
- `r-type_client`

---

## Features Roadmap

### Part 1 (Core Prototype - Delivery in Week 4)
-  Multi-threaded authoritative server
-  Graphical client (SFML/SDL/Raylib)
-  UDP binary protocol
-  Game engine with decoupled layers

### Part 2 (Advanced Topics - Delivery in Week 7)
Choose one or several:
-  Advanced Architecture (generic reusable engine)
-  Advanced Networking (lag compensation, prediction…)
-  Advanced Gameplay (bosses, new levels, tools)

---

## Documentation
- [Architecture](./docs/architecture/ARCHITECTURE.md)
- [Protocol](./docs/protocol/PROTOCOL.md)

---

## Authors
- Raphael Guerin
- Alexis Constantinopoulos
- Liza Goulmot
- Maryse Katary
License: MIT