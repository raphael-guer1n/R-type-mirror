/**
 * @file Server.cpp
 * @brief Implementation of the main server logic for the R-Type game.
 *
 * This file contains the core server-side logic, including entity management,
 * network communication, game tick handling, system registration, and enemy spawning.
 * The server uses an ECS (Entity Component System) architecture to manage game entities
 * and their behaviors, and communicates with clients via UDP sockets.
 *
 * Main functionalities:
 * - Registers and manages game components and systems.
 * - Handles player connections and spawns player entities.
 * - Processes network inputs from clients to update player states.
 * - Spawns enemies and projectiles at regular intervals based on game ticks.
 * - Runs game logic and updates entity states each tick.
 * - Broadcasts game state snapshots to all connected players.
 *
 * Key classes and functions:
 * - server::server: Constructor, initializes server and registers components/systems.
 * - server::run: Main game loop, processes inputs, updates game state, and broadcasts snapshots.
 * - server::register_components: Registers all ECS components used in the game.
 * - server::setup_systems: Registers all ECS systems, including AI and collision handling.
 * - server::game_handler: Spawns enemies and handles game-specific logic per tick.
 * - server::broadcast_snapshot: Sends the current game state to all players.
 * - server::wait_for_players: Waits for player connections and spawns player entities.
 * - server::process_network_inputs: Handles incoming network packets and updates player states.
 * - server::spawn_player: Spawns a new player entity with default components.
 * - server::spawn_projectile: Spawns a projectile entity for a given owner.
 *
 */
#include "engine/ecs/Systems.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "engine/events/Events.hpp"
#include "server/ServerUtils.hpp"
#include "engine/profiling/Profiler.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <random>
#include <thread>

#include "common/Components_client.hpp"
#include "engine/ecs/Systems.hpp"
#include "server/Components_ai.hpp"
#include "server/EnemyConfig.hpp"
#include "common/Accessibility.hpp"
#include "server/System_ai.hpp"
#include "Server.hpp"
#include "server/Server.hpp"
#include "engine/ecs/Systems.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "server/ServerUtils.hpp"
#include "enemies/Crawler.hpp"
#include "enemies/Shooter.hpp"
#include "enemies/Boss.hpp"

using json = nlohmann::json;

using namespace serverutils;

server::server(unsigned short port)
: _netServer(port), _lobbyManager(_netServer)
{
}

server::~server()
{
  stop();
}

void server::run()
{
  _running = true;
  _netServer.start();

  _tickThread = std::thread(&server::tick_loop, this);
  while (_running) {
    _netServer.poll();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  if (_tickThread.joinable())
    _tickThread.join();
  _netServer.stop();
}

void server::stop()
{
    _running = false;
    _lobbyManager.shutdown();
}

void server::tick_loop()
{
    using namespace std::chrono_literals;
    const auto tick_duration = 16ms;

    while (_running)
    {
        auto start = std::chrono::steady_clock::now();
        _lobbyManager.tick_all();
        auto end = std::chrono::steady_clock::now();
        auto elapsed = end - start;
        if (elapsed < tick_duration)
          std::this_thread::sleep_for(tick_duration - elapsed);
    }
}
