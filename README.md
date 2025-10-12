# R-Type: A Game Engine that Roars!

This is our implementation of the **R-Type multiplayer game** for the B-CPP-500 project at **Epitech**.

The goal is twofold:
1. Implement a **network multiplayer version** of the legendary 90's Shoot'em Up, R-Type.
2. Build a **custom game engine** that demonstrates real-world architecture patterns, separation of concerns, and networking.

---

## Build and Run

### Requirements
- C++17 compatible compiler (GCC/Clang on Linux, MSVC on Windows)
- [CMake](https://cmake.org/) (>= 3.18 recommended)
- [vcpkg](https://github.com/microsoft/vcpkg) (manifest mode, vcpkg.json provided)

### Quick build

- Linux/macOS
	- Run at the repository root:
		- `./build.sh`
	- This configures CMake with the vcpkg toolchain and builds at the root of the project

- Windows
	- Run at the repository root:
		- `build.bat`
	- This generates a Visual Studio 2022 x64 solution, uses the vcpkg toolchain, and builds into `Release`

Built binaries (depending on configuration):

**Linux / MacOS**

at the root of the project :

- `r-type_server`
- `r-type_client`

**Windows**

in the `Release\` folder :

- `r-type_server.exe`
- `r-type_client.exe`

### Using vcpkg

This project uses vcpkg in manifest mode via `vcpkg.json`.

Minimal setup:
- Clone vcpkg: `git clone https://github.com/microsoft/vcpkg.git`
- Bootstrap vcpkg:
	- Linux/macOS: `./vcpkg/bootstrap-vcpkg.sh`
	- Windows: `vcpkg\bootstrap-vcpkg.bat`

That’s it. The provided build scripts already pass the vcpkg toolchain file to CMake.

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
- [Comparative study for choosen technos](./docs/comparative_study.md)

---

## Authors
- Raphael Guerin
- Alexis Constantinopoulos
- Liza Goulmot
- Maryse Katary
- Kevin Poly
