#include "server/Server.hpp"
#include "engine/ecs/Systems.hpp"
#include "engine/ecs/Systems_client_sfml.hpp"
#include "engine/ecs/Components_client.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <random>

server::server(asio::io_context &ctx, unsigned short port)
    : _socket(ctx, port), _io(ctx), _port(port)
{
    std::random_device rd; // obtain a random number from hardware

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

    auto &positions = _registry.get_components<component::position>();
    auto &velocities = _registry.get_components<component::velocity>();
    auto &drawables = _registry.get_components<component::drawable>();
    auto &hitboxes = _registry.get_components<component::hitbox>();
    auto &controllables = _registry.get_components<component::controllable>();
    auto &collisions = _registry.get_components<component::collision_state>();
    auto &healths = _registry.get_components<component::health>();
    auto &damages = _registry.get_components<component::damage>();
    auto &despawns = _registry.get_components<component::despawn_tag>();
    auto &spawns = _registry.get_components<component::spawn_request>();
    auto &kinds = _registry.get_components<component::entity_kind>();
    auto &owners = _registry.get_components<component::controlled_by>();
    auto &locals = _registry.get_components<component::local_player_tag>();

    setup_systems();
}

void server::run()
{
    wait_for_players();
    std::cout << "All players connected!\n";

    using clock = std::chrono::steady_clock;
    const auto tick_duration = std::chrono::milliseconds(16);

    while (_running)
    {
        auto frame_start = clock::now();

        // Process
        process_network_inputs();
        game_handler();
        _registry.run_systems();
        broadcast_snapshot();
        _tick++;

        // Frame pacing
        auto frame_end = clock::now();
        auto elapsed = frame_end - frame_start;
        if (elapsed < tick_duration)
            std::this_thread::sleep_for(tick_duration - elapsed);
    }
}

void server::stop() { _running = false; }

void server::setup_systems()
{
    _registry.add_system<component::position, component::velocity>(position_system);
    _registry.add_system<component::velocity, component::controllable>(control_system);
    _registry.add_system<component::health, component::despawn_tag, component::damage>(health_system);
    _registry.add_system<component::despawn_tag>(despawn_system);
    _registry.add_system<component::spawn_request>(spawn_system);

    _registry.add_system<component::position, component::hitbox>(
        [this](engine::registry &reg,
               engine::sparse_array<component::position> &positions,
               engine::sparse_array<component::hitbox> &hitboxes)
        {
            auto &collisions = _registry.get_components<component::collision_state>();
            auto &velocities = _registry.get_components<component::velocity>();
            auto &kinds      = _registry.get_components<component::entity_kind>();
            auto &damages    = _registry.get_components<component::damage>();
            auto &cooldowns  = _registry.get_components<component::damage_cooldown>();

            std::vector<bool> newCollided(collisions.size(), false);

            // Resolve blocking against decor
            auto resolve_block = [&](std::size_t p, std::size_t wall)
            {
                if (p < collisions.size() && collisions[p])
                {
                    collisions[p]->collided = true;
                    auto &ppos = positions[p].value();
                    const auto &phb = hitboxes[p].value();
                    const auto &opos = positions[wall].value();
                    const auto &ohb  = hitboxes[wall].value();

                    float ax1 = ppos.x + phb.offset_x;
                    float ay1 = ppos.y + phb.offset_y;
                    float ax2 = ax1 + phb.width;
                    float ay2 = ay1 + phb.height;
                    float bx1 = opos.x + ohb.offset_x;
                    float by1 = opos.y + ohb.offset_y;
                    float bx2 = bx1 + ohb.width;
                    float by2 = by1 + ohb.height;

                    float overlapX = std::min(ax2, bx2) - std::max(ax1, bx1);
                    float overlapY = std::min(ay2, by2) - std::max(ay1, by1);

                    if (overlapX > 0.f && overlapY > 0.f)
                    {
                        float acx = (ax1 + ax2) * 0.5f;
                        float bcx = (bx1 + bx2) * 0.5f;
                        float acy = (ay1 + ay2) * 0.5f;
                        float bcy = (by1 + by2) * 0.5f;

                        if (overlapX < overlapY)
                        {
                            if (acx < bcx)
                                ppos.x -= overlapX;
                            else
                                ppos.x += overlapX;
                            if (p < velocities.size() && velocities[p])
                                velocities[p]->vx = 0.f;
                        }
                        else
                        {
                            if (acy < bcy)
                                ppos.y -= overlapY;
                            else
                                ppos.y += overlapY;
                            if (p < velocities.size() && velocities[p])
                                velocities[p]->vy = 0.f;
                        }
                    }
                }
            };

            // Apply damage with cooldown
            auto apply_damage_with_cooldown = [&](std::size_t p)
            {
                if (p >= damages.size())
                    return;
                uint32_t last = (p < cooldowns.size() && cooldowns[p]) ? cooldowns[p]->last_hit_tick : 0;
                if (_tick > last + 60) // 60 frame cooldown
                {
                    if (!damages[p])
                        reg.add_component(reg.entity_from_index(p), component::damage{1});
                    else
                        damages[p]->amount += 1;

                    if (p < cooldowns.size() && cooldowns[p])
                        cooldowns[p]->last_hit_tick = _tick;
                    else
                        reg.add_component(reg.entity_from_index(p), component::damage_cooldown{_tick});

                    if (p < collisions.size() && collisions[p])
                    {
                        collisions[p]->collided = true; // mark for rendering purple
                    }
                }
            };

            // Collision system
            hitbox_system(reg, positions, hitboxes,
                          [&](std::size_t i, std::size_t j)
                          {
                              auto kindI = (i < kinds.size() && kinds[i]) ? kinds[i].value() : component::entity_kind::unknown;
                              auto kindJ = (j < kinds.size() && kinds[j]) ? kinds[j].value() : component::entity_kind::unknown;

                              if (kindI == component::entity_kind::player && kindJ == component::entity_kind::decor)
                                  resolve_block(i, j);
                              if (kindJ == component::entity_kind::player && kindI == component::entity_kind::decor)
                                  resolve_block(j, i);

                              if (kindI == component::entity_kind::player && kindJ == component::entity_kind::enemy)
                              {
                                  apply_damage_with_cooldown(i);
                                  newCollided[i] = true;
                              }
                              if (kindJ == component::entity_kind::player && kindI == component::entity_kind::enemy)
                              {
                                  apply_damage_with_cooldown(j);
                                  newCollided[j] = true;
                              }
                          });

            // Update collision states
            for (size_t idx = 0; idx < collisions.size(); ++idx)
            {
                if (collisions[idx])
                    collisions[idx]->collided = newCollided[idx];
            }
        });
}

void server::wait_for_players()
{
    std::cout << "Waiting for 4 players..." << std::endl;

    while (_players.size() < 1)
    {
        // std::cout << "Currently " << _players.size() << " players connected." << std::endl;
        asio::ip::udp::endpoint sender;
        auto pkt_opt = _socket.receive(sender);
        if (pkt_opt)
        {
            auto [hdr, payload] = *pkt_opt;
            if (hdr.type == CONNECT_REQ)
            {
                PlayerInfo pi{sender, _registry.spawn_entity()};
                auto eid = pi.entityId;
                std::cout << "Spawned player entity: " << eid << " for " << sender << "\n";
                // Add components to the new player entity
                _registry.add_component(pi.entityId, component::position{100, 100});
                _registry.add_component(pi.entityId, component::velocity{0, 0});
                _registry.add_component(pi.entityId, component::hitbox{40, 40});
                _registry.add_component(pi.entityId, component::controllable{});
                _registry.add_component(pi.entityId, component::collision_state{false});
                _registry.add_component(pi.entityId, component::health{10});
                _registry.add_component(pi.entityId, component::damage{0});
                _registry.add_component(pi.entityId, component::entity_kind::player);
                _registry.add_component(pi.entityId, component::controlled_by{1}); // exemple: clientId 1
                _registry.add_component(pi.entityId, component::local_player_tag{});
                _registry.add_component(pi.entityId, component::drawable{sf::Vector2f{40.f, 40.f}, sf::Color::Green});
                _registry.add_component(pi.entityId, component::damage_cooldown{0});

                _players.push_back(pi);
                // Ack back
                ConnectAck ack{1234, 60, static_cast<uint32_t>(eid)}; // serverId, tickRate
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
                // Find player by endpoint
                for (auto &p : _players) {
                    if (p.endpoint == sender && p.entityId == input.clientId) {
                        std::cout << "key " << input.keys + 48 << ", id " << input.clientId << ", timestamp: " << input.tick << std::endl;
                        auto &vel = *_registry.get_components<component::velocity>()[p.entityId];
                        vel.vx = (input.keys & 0x01) ? -10 : (input.keys & 0x02) ? 10 : 0;
                        vel.vy = (input.keys & 0x04) ? -10 : (input.keys & 0x08) ? 10 : 0;
                    }
                }
            }
        }
    }
}

void server::game_handler()
{
    if (_tick % 20 == 0)
    {
        auto new_enemy = _registry.spawn_entity();
        int posX = std::uniform_int_distribution<int>(0, 1920)(_gen);
        int posY = std::uniform_int_distribution<int>(0, 1080)(_gen);
        _registry.add_component(new_enemy, component::position{(float)posX, (float)posY});
        _registry.add_component(new_enemy, component::velocity{0.f, 0.f});
        _registry.add_component(new_enemy, component::hitbox{40.f, 40.f});
        _registry.add_component(new_enemy, component::entity_kind::enemy);
        
        // auto wall = _registry.spawn_entity();
        // int wallX = std::uniform_int_distribution<int>(0, 1920)(_gen);
        // int wallY = std::uniform_int_distribution<int>(0, 1080)(_gen);
        // _registry.add_component(wall, component::position{(float)wallX, (float)(wallY)});
        // _registry.add_component(wall, component::velocity{0.f, 0.f});
        // _registry.add_component(wall, component::hitbox{60.f, 30.f});
        // _registry.add_component(wall, component::entity_kind::decor);
    }
}

void server::broadcast_snapshot()
{
    auto &positions = _registry.get_components<component::position>();
    auto &kinds = _registry.get_components<component::entity_kind>();
    auto &collisions = _registry.get_components<component::collision_state>();

    std::vector<EntityState> states;
    for (std::size_t i = 0; i < positions.size(); ++i)
    {
        if (positions[i])
        {
            EntityState es{};
            es.entityId = i;
            es.x = positions[i]->x;
            es.y = positions[i]->y;
            es.vx = 0;
            es.vy = 0;
            es.hp = 100;
            if (i < kinds.size() && kinds[i])
                es.type = static_cast<uint8_t>(kinds[i].value());
            else
                es.type = static_cast<uint8_t>(component::entity_kind::unknown);
            es.collided = (i < collisions.size() && collisions[i] && collisions[i]->collided) ? 1 : 0;
            states.push_back(es);
        }
    }
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