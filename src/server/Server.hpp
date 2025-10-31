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
#include "engine/network/NetServer.hpp"

#define PLAYER_SPEED 400.0f
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define MAX_PLAYERS 2
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
    server(unsigned short port = 4242);
    void run();

private:
    // Initialization / registration
    void register_components();
    void setup_systems();

    // Network
    void on_packet_received(const engine::net::Endpoint& sender,
        const PacketHeader& hdr, const std::vector<uint8_t>& payload);
    void handle_connect(const engine::net::Endpoint& sender);
    void handle_input(const engine::net::Endpoint& sender,
        const std::vector<uint8_t>& payload);

    // Sub-registrations (split from setup_systems)
    void register_health_and_spawn_systems();
    void register_projectile_movement_system();
    void register_gravity_system();
    void register_collision_system();
    void register_bounds_system();
    void register_area_effect_system();

    // Game loop phases
    void update_game_logic();
    void update_spawns_and_events();
    void broadcast_snapshot();
    void broadcast_game_over(uint32_t winnerEntityId);
    void check_game_over();

    // Spawning helpers
    engine::entity_t spawn_player(engine::net::Endpoint endpoint, std::size_t index);

    // Internal utility: ensure an entity is scheduled for removal by setting/adding despawn_tag.
    // Centralises logic so systems never call kill_entity directly (uniform ECS pipeline).
private:
    bool _running = false;
    bool _ready = false;
    engine::registry _registry;

    engine::net::NetServer _netServer;
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

    // Input edge state per player
    std::unordered_map<uint32_t, bool> _prevSpace;
    std::unordered_map<uint32_t, bool> _prevC;
    std::unordered_map<uint32_t, uint32_t> _pressTick;
};