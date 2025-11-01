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

struct PlayerInfo {
    engine::net::Endpoint endpoint;
    engine::entity_t entityId;
};

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

        // Spawning helpers
        engine::entity_t spawn_player(engine::net::Endpoint endpoint, std::size_t index);
    private:
        bool _running = false;
        engine::registry _registry;

        engine::net::NetServer &_server;
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
