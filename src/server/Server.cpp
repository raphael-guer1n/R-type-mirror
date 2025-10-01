#include <thread>
#include <chrono>
#include <iostream>
#include <random>
#include <fstream>
#include <nlohmann/json.hpp>

#include "server/Server.hpp"
#include "common/Systems.hpp"
#include "common/Systems_client_sfml.hpp"
#include "common/Components_client.hpp"
#include "server/Components_ai.hpp"
#include "server/System_ai.hpp"
#include "server/EnemyConfig.hpp"

using json = nlohmann::json;

server::server(asio::io_context &ctx, unsigned short port)
    : _socket(ctx, port), _io(ctx), _port(port)
{

    // Default components
    _registry.register_component<component::position>();
    _registry.register_component<component::velocity>();
    _registry.register_component<component::drawable>();
    _registry.register_component<component::hitbox>();
    _registry.register_component<component::controllable>();
    _registry.register_component<component::collision_state>();
    _registry.register_component<component::health>();
    _registry.register_component<component::damage>();
    _registry.register_component<component::despawn_tag>();
    _registry.register_component<component::spawn_request>();
    _registry.register_component<component::entity_kind>();
    _registry.register_component<component::controlled_by>();
    _registry.register_component<component::local_player_tag>();
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
            game_handler();
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
    _registry.add_system<component::position, component::velocity>(position_system);
    _registry.add_system<component::velocity, component::controllable>(control_system);
    _registry.add_system<component::health, component::despawn_tag, component::damage>(health_system);

    _registry.add_system<component::despawn_tag>(
        [this](engine::registry &reg,
               engine::sparse_array<component::despawn_tag> &despawns)
        {
            for (auto &&[i, d] : indexed_zipper(despawns))
            {
                if (d.now)
                {
                    _live_entities.erase(static_cast<uint32_t>(i));
                    reg.kill_entity(reg.entity_from_index(i));
                }
            }
        });

    _registry.add_system<component::spawn_request>(spawn_system);

    // Collision/hitbox
    _registry.add_system<component::position, component::hitbox>(
        [this](engine::registry &reg,
               engine::sparse_array<component::position> &positions,
               engine::sparse_array<component::hitbox> &hitboxes)
        {
            auto &collisions = _registry.get_components<component::collision_state>();
            auto &velocities = _registry.get_components<component::velocity>();
            auto &kinds = _registry.get_components<component::entity_kind>();
            auto &damages = _registry.get_components<component::damage>();
            auto &cooldowns = _registry.get_components<component::damage_cooldown>();

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
            }
        });

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

    std::vector<EntityState> states;
    states.reserve(50);

    for (uint32_t entityId : _live_entities)
    {
        if (states.size() >= 40)
            break;
        size_t idx = static_cast<size_t>(entityId);

        if (idx < positions.size() && positions[idx])
        {
            EntityState es{};
            es.entityId = entityId;
            es.x = positions[idx]->x;
            es.y = positions[idx]->y;
            es.vx = 0;
            es.vy = 0;
            es.hp = 100;
            if (idx < kinds.size() && kinds[idx])
                es.type = static_cast<uint8_t>(kinds[idx].value());
            else
                es.type = static_cast<uint8_t>(component::entity_kind::unknown);
            es.collided = (idx < collisions.size() && collisions[idx] && collisions[idx]->collided) ? 1 : 0;
            states.push_back(es);
        }
    }

    if (states.empty())
        return;

    Snapshot snap{_tick, static_cast<uint16_t>(states.size())};
    std::vector<uint8_t> buf(sizeof(Snapshot) + sizeof(EntityState) * states.size());
    std::memcpy(buf.data(), &snap, sizeof(Snapshot));
    std::memcpy(buf.data() + sizeof(Snapshot), states.data(),
                sizeof(EntityState) * states.size());

    PacketHeader hdr{SNAPSHOT, static_cast<uint16_t>(buf.size()), _tick};
    for (auto &p : _players)
    {
        _socket.send(hdr, buf, p.endpoint);
    }
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
                PlayerInfo pi{sender, _registry.spawn_entity()};
                _live_entities.insert(static_cast<uint32_t>(pi.entityId));
                auto eid = pi.entityId;
                std::cout << "Spawned player entity: " << eid << " for " << sender << "\n";
                _registry.add_component(pi.entityId, component::position{100, 100});
                _registry.add_component(pi.entityId, component::velocity{0, 0});
                _registry.add_component(pi.entityId, component::hitbox{40, 40});
                _registry.add_component(pi.entityId, component::controllable{});
                _registry.add_component(pi.entityId, component::collision_state{false});
                _registry.add_component(pi.entityId, component::health{10});
                _registry.add_component(pi.entityId, component::damage{0});
                _registry.add_component(pi.entityId, component::entity_kind::player);
                _registry.add_component(pi.entityId, component::controlled_by{1});
                _registry.add_component(pi.entityId, component::local_player_tag{});
                _registry.add_component(pi.entityId, component::drawable{sf::Vector2f{40.f, 40.f}, sf::Color::Green});
                _registry.add_component(pi.entityId, component::damage_cooldown{0});

                _players.push_back(pi);
                ConnectAck ack{1234, 60, static_cast<uint32_t>(eid)};
                PacketHeader h{CONNECT_ACK,
                               static_cast<uint16_t>(sizeof(ConnectAck)),
                               0};
                std::vector<uint8_t> buf(sizeof(ConnectAck));
                std::memcpy(buf.data(), &ack, sizeof(ConnectAck));
                _socket.send(h, buf, sender);
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
                            vel.vx = (input.keys & 0x01) ? -10 : (input.keys & 0x02) ? 10
                                                                                     : 0;
                            vel.vy = (input.keys & 0x04) ? -10 : (input.keys & 0x08) ? 10
                                                                                     : 0;
                        }
                        break;
                    }
                }
            }
        }
    }
}
