/**
 * @file GameLogic.hpp
 * @brief Declares the GameLogic class responsible for managing the core server-side game simulation.
 *
 * This file defines the GameLogic class, which encapsulates the authoritative simulation
 * of gameplay for the R-Type server. It handles player entities, physics updates,
 * projectile and enemy spawning, input processing, and synchronization of game state
 * via network snapshots.
 *
 * The class runs independently for each active lobby and maintains its own registry,
 * tick counter, and live entities list.
 */
#pragma once
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <random>
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Components.hpp"
#include "common/Packets.hpp"
#include "engine/network/IoContext.hpp"
#include "engine/network/UdpSocket.hpp"
#include "engine/network/Endpoint.hpp"
#include "engine/network/NetServer.hpp"
#include "LevelManager.hpp"

#define PLAYER_SPEED 400.0f
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define MAX_PLAYERS 2

/**
 * @struct PlayerInfo
 * @brief Represents a player connected to the server.
 *
 * Each player is uniquely identified by their network endpoint and is associated
 * with an in-game entity ID.
 */
struct PlayerInfo {
    engine::net::Endpoint endpoint;
    engine::entity_t entityId;
};


/**
 * @class GameLogic
 * @brief Handles all server-side game logic, including entity management, physics, and network updates.
 *
 * The GameLogic class runs the core simulation loop of a single lobby instance.
 * It manages players, spawns, collision updates, and level progression.
 * It also handles player input received over the network and broadcasts synchronized
 * snapshots of the world state to all connected clients.
 *
 * ### Responsibilities:
 * - Maintain the ECS registry and manage all entity components.
 * - Process player inputs and translate them into entity updates.
 * - Spawn entities such as projectiles, enemies, and levels.
 * - Handle game progression, tick-based updates, and win/loss conditions.
 * - Broadcast snapshots of the game state to clients at a fixed tick rate.
 *
 * @note This class is designed to be deterministic per lobby and must not share state between games.
 */
class GameLogic
{
    public:
        GameLogic(engine::net::NetServer& _server);
        ~GameLogic() = default;
    public:
        void update_game_logic();
        void update_spawns_and_events();
        void broadcast_snapshot();
        void handle_input(const engine::net::Endpoint &sender,
            const std::vector<uint8_t> &payload);
        std::vector<PlayerInfo>& getPlayers() {return _players;}
        engine::entity_t spawn_player(engine::net::Endpoint endpoint, std::size_t index);
        std::unordered_set<uint32_t>& getLiveEntities() {return _live_entities;}
        uint32_t &getTick() {return _tick;}
        bool &getRunning() {return _running;}
    private:
        // Initialization / registration
        void register_components();
        void setup_systems();

        // Sub-registrations (split from setup_systems)
        void register_health_and_spawn_systems();
        void register_projectile_movement_system();
        void register_gravity_system();
        void register_collision_system();
        void register_bounds_system();
        void register_area_effect_system();

        // Game loop phases
        void broadcast_game_over(uint32_t winnerEntityId);
        void check_game_over();

    private:
        bool _running = false;
        engine::registry _registry;

        engine::net::NetServer &_server;
        std::unordered_set<uint32_t> _live_entities;
        std::vector<PlayerInfo> _players;
        std::unique_ptr<LevelManager> _levelManager;

        uint32_t _tick = 0;
        std::random_device rd;
        std::mt19937 _gen{rd()};

        // Input edge state per player
        std::unordered_map<uint32_t, bool> _prevSpace;
        std::unordered_map<uint32_t, bool> _prevC;
        std::unordered_map<uint32_t, uint32_t> _pressTick;
};
