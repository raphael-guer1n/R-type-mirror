#include "server/Server.hpp"
#include "common/Systems.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <random>

server::server(asio::io_context &ctx, unsigned short port)
    : _socket(ctx, port), _io(ctx), _port(port)
{
    std::random_device rd; // obtain a random number from hardware

    _registry.register_component<component::position>();
    _registry.register_component<component::controllable>();
    _registry.register_component<component::velocity>();
    setup_systems();
}

void server::run()
{
    wait_for_players();
    std::cout << "All players connected!" << std::endl;

    using clock = std::chrono::steady_clock;
    auto tick_duration = std::chrono::milliseconds(16);
    auto next_tick = clock::now();

    while (_running)
    {
        process_network_inputs();
        game_handler();
        _registry.run_systems();
        broadcast_snapshot();
        _tick++;
        next_tick += tick_duration;
        std::this_thread::sleep_until(next_tick);
    }
}

void server::stop() { _running = false; }

void server::setup_systems()
{
    _registry.add_system<component::position, component::velocity>(position_system);
    _registry.add_system<component::velocity, component::controllable>(control_system);
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
                _registry.add_component(pi.entityId, component::controllable{});
                _registry.add_component(pi.entityId, component::position{100, 100});
                _registry.add_component(pi.entityId, component::velocity{0, 0});
                _players.push_back(pi);

                // Ack back
                ConnectAck ack{1234, 60}; // serverId, tickRate
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
                std::cout << "key " << input.keys << ", id " << input.clientId << ", timestamp: " << input.tick << std::endl;
                // Find player by endpoint
                for (auto &p : _players)
                {
                    if (p.endpoint == sender)
                    {
                        auto &vel =
                            *_registry.get_components<component::velocity>()[p.entityId];
                        vel.vx = (input.keys & 0x01) ? -3 : (input.keys & 0x02) ? 3
                                                                                : 0;
                        vel.vy = (input.keys & 0x04) ? -3 : (input.keys & 0x08) ? 3
                                                                                : 0;
                    }
                }
            }
        }
    }
}

void server::game_handler()
{
    if (_tick % 60 == 0) {
        auto new_enemy = _registry.spawn_entity();

        int posX = std::uniform_int_distribution<int>(0, 1920)(_gen);
        int posY = std::uniform_int_distribution<int>(0, 1080)(_gen);

        _registry.add_component(new_enemy, component::position{(float)posX, (float)posY});
        _registry.add_component(new_enemy, component::velocity{0.f, 0.f});
    }
}

void server::broadcast_snapshot()
{
    auto &positions = _registry.get_components<component::position>();

    std::vector<EntityState> states;
    for (std::size_t i = 0; i < positions.size(); ++i)
    {
        if (positions[i])
        {
            EntityState es{};
            es.entityId = i;
            es.x = static_cast<float>(positions[i]->x);
            es.y = static_cast<float>(positions[i]->y);
            es.vx = 0;
            es.vy = 0;
            es.type = 0;
            es.hp = 100;
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