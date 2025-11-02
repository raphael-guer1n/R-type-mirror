#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <random>
#include "LevelManager.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Components.hpp"
#include "common/Packets.hpp"
#include "engine/network/IoContext.hpp"
#include "engine/network/UdpSocket.hpp"
#include "engine/network/Endpoint.hpp"
#include "engine/network/NetServer.hpp"
#include "Lobby/Lobby.hpp"
#include "Lobby/LobbyManager.hpp"
/**
 * @class server
 * @brief Main server class for managing game state, networking, and player entities.
 *
 * This class encapsulates the core logic for running the game server, including player registration,
 * network communication, game loop management, and entity handling using an ECS (Entity Component System) architecture.
 *
 * @note Networking is provided via engine wrappers; Asio is encapsulated inside the engine.
 *
 * @section Responsibilities
 * - Accepts and manages player connections via UDP.
 * - Handles game loop phases: waiting for players, processing inputs, updating game state, and broadcasting snapshots.
 * - Spawns and manages player and projectile entities.
 * - Centralizes entity removal logic to maintain ECS pipeline integrity.
 *
 */
class server
{
public:
    server(unsigned short port = 4242);
    ~server();
    void run();
    void stop();

private:
    void tick_loop();
private:
    bool _running = false;
    bool _ready = false;
    engine::registry _registry;

    engine::net::NetServer _netServer;
    LobbyManager _lobbyManager;
    std::thread _tickThread;

    std::vector<PlayerInfo> _players;

    uint32_t _tick = 0;
    uint32_t _highscore = 0;
};
