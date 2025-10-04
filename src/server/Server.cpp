#include "server/Server.hpp"
#include "common/Systems.hpp"
#include "common/Systems_client_sdl.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "server/ServerUtils.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <random>
#include <fstream>
#include <nlohmann/json.hpp>

#include "server/Server.hpp"
#include "common/Systems.hpp"
#include "common/Systems_client_sdl.hpp"
#include "common/Components_client.hpp"
#include "server/Components_ai.hpp"
#include "server/System_ai.hpp"
#include "server/EnemyConfig.hpp"

using json = nlohmann::json;

using namespace serverutils;

server::server(asio::io_context &ctx, unsigned short port)
    : _socket(ctx, port), _io(ctx), _port(port)
{
    register_components();
    setup_systems();
}

// Default components
void server::register_components()
{
    _registry.register_component<component::position>();
    _registry.register_component<component::velocity>();
    _registry.register_component<component::hitbox>();
    _registry.register_component<component::controllable>();
    _registry.register_component<component::collision_state>();
    _registry.register_component<component::health>();
    _registry.register_component<component::damage>();
    _registry.register_component<component::spawn_request>();
    _registry.register_component<component::entity_kind>();
    _registry.register_component<component::controlled_by>();
    _registry.register_component<component::damage_cooldown>();

    // AI related
    _registry.register_component<component::ai_controller>();
    _registry.register_component<component::spell>();
    _registry.register_component<component::spellbook>();
    _registry.register_component<component::boss_phase>();

    setup_systems();
}

void server::run()
{
    wait_for_players();
    std::cout << "All players connected!\n";

    using clock = std::chrono::steady_clock;
    const auto tick_duration = std::chrono::milliseconds(16);
    auto last_tick = clock::now();

    while (_running)
    {
        process_network_inputs();

        auto now = clock::now();
        if (now - last_tick >= tick_duration)
        {
            auto& positions = _registry.get_components<component::position>();
            auto& velocities = _registry.get_components<component::velocity>();
            auto& controls = _registry.get_components<component::controllable>();
            game_handler();
            position_system(_registry, positions, velocities, 1.0f / 60.0f);
            _registry.run_systems();
            broadcast_snapshot();
            _tick++;

            last_tick += tick_duration;

            if (now - last_tick >= tick_duration)
            {
                last_tick = now;
            }
        }
    }
}

void server::stop() { _running = false; }

void server::setup_systems()
{
    _registry.add_system<component::health, component::damage>(health_system);
    _registry.add_system<component::spawn_request>(spawn_system);

    // Collision/hitbox
    _registry.add_system<component::position, component::projectile_tag>([this](engine::registry &reg,
                                                                                engine::sparse_array<component::position> &positions,
                                                                                engine::sparse_array<component::projectile_tag> &projectiles)
                                                                         {
        std::vector<engine::entity_t> toKill;
        for (auto &&[i, pos, proj] : indexed_zipper(positions, projectiles)) {
            pos.x += proj.dirX * proj.speed;
            pos.y += proj.dirY * proj.speed;
            if (proj.lifetime > 0) --proj.lifetime;
            if (proj.lifetime <= 0) toKill.push_back(reg.entity_from_index(i));
        }
        for (auto e : toKill) {
            _live_entities.erase(static_cast<uint32_t>(e));
            reg.kill_entity(e);
        } });

    _registry.add_system<component::position, component::hitbox>([this](engine::registry &reg,
                                                                        engine::sparse_array<component::position> &positions,
                                                                        engine::sparse_array<component::hitbox> &hitboxes)
                                                                 {
        auto &collisions = _registry.get_components<component::collision_state>();
        auto &velocities = _registry.get_components<component::velocity>();
        auto &kinds = _registry.get_components<component::entity_kind>();
        auto &damages = _registry.get_components<component::damage>();
        auto &cooldowns = _registry.get_components<component::damage_cooldown>();
        auto &projectiles = _registry.get_components<component::projectile_tag>();
        std::vector<bool> newCollided(collisions.size(), false);

            auto apply_damage_with_cooldown = [&](std::size_t p)
            {
                if (p >= damages.size())
                    return;
                uint32_t last =
                    (p < cooldowns.size() && cooldowns[p]) ? cooldowns[p]->last_hit_tick : 0;
                if (_tick > last + 60)
                {
                    if (!damages[p])
                        reg.add_component<component::damage>(reg.entity_from_index(p),
                                                             component::damage{1});
                    else
                        damages[p]->amount += 1;
                    if (p < cooldowns.size() && cooldowns[p])
                        cooldowns[p]->last_hit_tick = _tick;
                    else
                        reg.add_component<component::damage_cooldown>(
                            reg.entity_from_index(p), component::damage_cooldown{_tick});
                    if (p < collisions.size() && collisions[p])
                        collisions[p]->collided = true;
                }
            };

            hitbox_system(reg, positions, hitboxes,
                          [&](std::size_t i, std::size_t j)
                          {
                              auto kindI = (i < kinds.size() && kinds[i])
                                               ? kinds[i].value()
                                               : component::entity_kind::unknown;
                              auto kindJ = (j < kinds.size() && kinds[j])
                                               ? kinds[j].value()
                                               : component::entity_kind::unknown;

                              if (kindI == component::entity_kind::player &&
                                  kindJ == component::entity_kind::enemy)
                              {
                                  apply_damage_with_cooldown(i);
                                  newCollided[i] = true;
                              }
                              if (kindJ == component::entity_kind::player &&
                                  kindI == component::entity_kind::enemy)
                              {
                                  apply_damage_with_cooldown(j);
                                  newCollided[j] = true;
                              }
                          });

            for (size_t idx = 0; idx < collisions.size(); ++idx)
            {
                if (collisions[idx])
                    collisions[idx]->collided = newCollided[idx];
            } });

    systems::init_ai_behaviors();
    _registry.add_system<component::position, component::velocity, component::ai_controller>(
        [this](auto &reg, auto &pos, auto &vel, auto &ai)
        {
            systems::enemy_ai_system(reg, pos, vel, ai, _tick);
        });

    _registry.add_system<component::health, component::ai_controller, component::boss_phase>(
        systems::boss_phase_system);
}

void server::game_handler()
{
    // Zigzag enemy
    if (_tick % 30 == 0) {
        for (auto &p : _players) {
            auto proj = spawn_projectile(p.entityId);
            _live_entities.insert(static_cast<uint32_t>(proj));
        }
    }
    if (_tick % 200 == 0)
    {
        try
        {
            EnemyConfig cfg = EnemyConfig::load_enemy_config("configs/enemy/crawler.json");
            auto e = _registry.spawn_entity();
            _live_entities.insert((uint32_t)e);

            _registry.add_component(e, component::position{1820, 300});
            _registry.add_component(e, component::velocity{0, 0});
            _registry.add_component<component::hitbox>(e, std::move(cfg.hitbox));
            _registry.add_component(e, component::entity_kind::enemy);
            _registry.add_component(e, component::collision_state{false});
            _registry.add_component(e, component::health{(uint8_t)cfg.hp});

            component::ai_controller ai;
            ai.behavior = cfg.behavior;
            ai.speed = cfg.speed;
            _registry.add_component<component::ai_controller>(e, std::move(ai));

            if (!cfg.spells.empty())
            {
                component::spellbook sb;
                sb.spells = cfg.spells;
                _registry.add_component<component::spellbook>(e, std::move(sb));
            }
            std::cout << "Spawned Zigzag enemy\n";
        }
        catch (std::exception &ex)
        {
            std::cerr << "Failed to load zigzag enemy: " << ex.what() << "\n";
        }
    }

    // Shooter enemy
    if (_tick % 400 == 0)
    {
        try
        {
            EnemyConfig cfg = EnemyConfig::load_enemy_config("configs/enemy/shooter.json");
            auto e = _registry.spawn_entity();
            _live_entities.insert((uint32_t)e);

            _registry.add_component(e, component::position{1820, 500});
            _registry.add_component(e, component::velocity{0, 0});
            _registry.add_component<component::hitbox>(e, std::move(cfg.hitbox));
            _registry.add_component(e, component::entity_kind::enemy);
            _registry.add_component(e, component::collision_state{false});
            _registry.add_component(e, component::health{(uint8_t)cfg.hp});
            component::ai_controller ai;
            ai.behavior = cfg.behavior;
            ai.speed = cfg.speed;
            _registry.add_component<component::ai_controller>(e, std::move(ai));
            if (!cfg.spells.empty())
            {
                component::spellbook sb;
                sb.spells = cfg.spells;
                _registry.add_component<component::spellbook>(e, std::move(sb));
            }
            std::cout << "Spawned Shooter enemy\n";
        }
        catch (std::exception &ex)
        {
            std::cerr << "Failed to load shooter enemy: " << ex.what() << "\n";
        }
    }
}

void server::broadcast_snapshot()
{
    auto &positions = _registry.get_components<component::position>();
    auto &kinds = _registry.get_components<component::entity_kind>();
    auto &collisions = _registry.get_components<component::collision_state>();
    auto &healths = _registry.get_components<component::health>();

    constexpr std::size_t SNAPSHOT_LIMIT = 80;
    std::vector<EntityState> states;
    states.reserve(50);
    std::unordered_set<uint32_t> inserted; // track inside this call

    SnapshotBuilderContext ctx{positions, kinds, collisions, healths};

    for (auto &pInfo : _players)
    {
        try_add_entity(static_cast<uint32_t>(pInfo.entityId), states, ctx, inserted, SNAPSHOT_LIMIT);
    }
    for (uint32_t entityId : _live_entities)
    {
        if (states.size() >= SNAPSHOT_LIMIT)
            break;
        size_t idx = static_cast<size_t>(entityId);
        try_add_entity(entityId, states, ctx, inserted, SNAPSHOT_LIMIT);
    }
    if (states.empty())
        return;

    Snapshot snap{_tick, static_cast<uint16_t>(states.size())};
    std::vector<uint8_t> buf(sizeof(Snapshot) + sizeof(EntityState) * states.size());
    std::memcpy(buf.data(), &snap, sizeof(Snapshot));
    std::memcpy(buf.data() + sizeof(Snapshot), states.data(), sizeof(EntityState) * states.size());
    PacketHeader hdr{SNAPSHOT, static_cast<uint16_t>(buf.size()), _tick};
    for (auto &p : _players)
        _socket.send(hdr, buf, p.endpoint);
}

void server::wait_for_players()
{
    std::cout << "Waiting for 4 players..." << std::endl;

    while (_players.size() < 1)
    {
        asio::ip::udp::endpoint sender;
        auto pkt_opt = _socket.receive(sender);
        if (pkt_opt)
        {
            auto [hdr, payload] = *pkt_opt;
            if (hdr.type == CONNECT_REQ)
            {
                std::size_t playerIndex = _players.size();
                float spawnX = 100.f;
                float spawnY = 100.f + 120.f * static_cast<float>(playerIndex);

                auto eid = spawn_player(sender, playerIndex);
                PlayerInfo pi{sender, eid};
                _live_entities.insert(static_cast<uint32_t>(eid));
                std::cout << "Spawned player entity: " << eid << " for " << sender << "\n";

                _players.push_back(pi);
                ConnectAck ack{1234, 60, static_cast<uint32_t>(eid)};
                PacketHeader h{CONNECT_ACK, static_cast<uint16_t>(sizeof(ConnectAck)), 0};
                std::vector<uint8_t> buf(sizeof(ConnectAck));
                std::memcpy(buf.data(), &ack, sizeof(ConnectAck));
                _socket.send(h, buf, sender);

                broadcast_snapshot();
            }
        }
    }
}

void server::process_network_inputs()
{
    asio::ip::udp::endpoint sender;
    while (auto pkt_opt = _socket.receive(sender))
    {
        auto [hdr, payload] = *pkt_opt;
        if (hdr.type == INPUT)
        {
            if (payload.size() >= sizeof(InputPacket))
            {
                InputPacket input{};
                std::memcpy(&input, payload.data(), sizeof(InputPacket));

                if (_live_entities.find(input.clientId) == _live_entities.end())
                {
                    continue;
                }

                for (auto &p : _players)
                {
                    if (p.endpoint == sender && p.entityId == input.clientId)
                    {
                        auto &velocities = _registry.get_components<component::velocity>();
                        if (static_cast<size_t>(p.entityId) < velocities.size() && velocities[p.entityId])
                        {
                            auto &vel = *velocities[p.entityId];
                            vel.vx = (input.keys & 0x01) ? -PLAYER_SPEED: (input.keys & 0x02) ? PLAYER_SPEED
                                                                                     : 0;
                            vel.vy = (input.keys & 0x04) ? -PLAYER_SPEED : (input.keys & 0x08) ? PLAYER_SPEED
                                                                                     : 0;
                        }
                        break;
                    }
                }
            }
        }
    }
}

engine::entity_t server::spawn_player(asio::ip::udp::endpoint endpoint, std::size_t index)
{
    float spawnX = 100.f;
    float spawnY = 100.f + 120.f * static_cast<float>(index);
    auto eid = engine::make_entity(
        _registry,
        component::position{spawnX, spawnY},
        component::velocity{0, 0},
        component::hitbox{40, 40},
        component::controllable{},
        component::collision_state{false},
        component::health{10},
        component::damage{0},
        component::entity_kind::player,
        component::controlled_by{static_cast<uint32_t>(index)},
        component::damage_cooldown{0});
    return eid;
}

engine::entity_t server::spawn_projectile(engine::entity_t owner)
{
    auto &positions = _registry.get_components<component::position>();
    auto &hitboxes = _registry.get_components<component::hitbox>();
    size_t idx = static_cast<size_t>(owner);
    if (idx >= positions.size() || !positions[idx])
        return owner; // invalid -> no spawn
    auto pos = positions[idx].value();
    float playerW = 0.f, playerH = 0.f;
    if (idx < hitboxes.size() && hitboxes[idx])
    {
        playerW = hitboxes[idx]->width;
        playerH = hitboxes[idx]->height;
    }
    constexpr float projectileW = 10.f;
    constexpr float projectileH = 10.f;
    float startX = pos.x + playerW + 4.f;
    float startY = pos.y + (playerH * 0.5f) - (projectileH * 0.5f);
    auto proj = engine::make_entity(
        _registry,
        component::position{startX, startY},
        component::hitbox{projectileW, projectileH},
        component::collision_state{false},
        component::entity_kind::projectile,
        component::projectile_tag{static_cast<uint32_t>(owner), 120, 1.f, 0.f, 25.f, 2},
        component::health{1});
    return proj;
}
