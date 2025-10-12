#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <random>
#include "engine/ecs/Registry.hpp"
#include "common/Components.hpp"
#include "common/Packets.hpp"
#include "engine/network/IoContext.hpp"
#include "engine/network/UdpSocket.hpp"
#include "engine/network/Endpoint.hpp"

#define PLAYER_SPEED 200.0f
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
 * @section Usage
 * Instantiate with an ASIO io_context and optional port, then call run() to start the server loop.
 *
 * @section Members
 * - _registry: ECS registry for managing entities and components.
 * - _socket: UDP socket for network communication.
 * - _players: List of connected players and their associated entities.
 * - _live_entities: Set of currently active entities.
 * - _tick: Current server tick for synchronization.
 * - _gen: Random number generator for entity spawning and game logic.
 */
class server
{
public:
    server(engine::net::IoContext &ctx, unsigned short port = 4242);
    void run();
    void stop();

private:
    // Initialization / registration
    void register_components();
    void setup_systems();

    // Game loop phases
    void wait_for_players();
    void process_network_inputs();
    void game_handler();
    void broadcast_snapshot();

    // Spawning helpers
    engine::entity_t spawn_player(engine::net::Endpoint endpoint, std::size_t index);
    engine::entity_t spawn_projectile(engine::entity_t owner);

    // Internal utility: ensure an entity is scheduled for removal by setting/adding despawn_tag.
    // Centralises logic so systems never call kill_entity directly (uniform ECS pipeline).
private:
    bool _running = true;
    engine::registry _registry;

    engine::net::UdpSocket _socket;
    engine::net::IoContext &_io;
    unsigned short _port;

    struct PlayerInfo
    {
        engine::net::Endpoint endpoint;
        engine::entity_t entityId;
    };

    std::unordered_set<uint32_t> _live_entities;
    std::vector<PlayerInfo> _players;

    uint32_t _tick = 0;

    std::random_device rd;
    std::mt19937 _gen{rd()};
};