#pragma once
#include <asio.hpp>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <random>
#include "engine/ecs/Registry.hpp"
#include "common/Components.hpp"
#include "common/Packets.hpp"
#include "engine/network/Udpsocket.hpp"

#define PLAYER_SPEED 400.0f

class server
{
public:
    server(asio::io_context &ctx, unsigned short port = 4242);
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
    engine::entity_t spawn_player(asio::ip::udp::endpoint endpoint, std::size_t index);
    engine::entity_t spawn_projectile(engine::entity_t owner);

    // Internal utility: ensure an entity is scheduled for removal by setting/adding despawn_tag.
    // Centralises logic so systems never call kill_entity directly (uniform ECS pipeline).
private:
    bool _running = true;
    engine::registry _registry;

    engine::net::UdpSocket _socket;
    asio::io_context &_io;
    unsigned short _port;

    struct PlayerInfo
    {
        asio::ip::udp::endpoint endpoint;
        engine::entity_t entityId;
    };

    std::unordered_set<uint32_t> _live_entities;
    std::vector<PlayerInfo> _players;

    uint32_t _tick = 0;

    std::random_device rd;
    std::mt19937 _gen{rd()}; // ✅ correct seeding
};