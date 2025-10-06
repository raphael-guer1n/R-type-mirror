#include <iostream>
#include <SDL.h>
#include <asio.hpp>
#include "Rtype.hpp"
#include "engine/renderer/Vectors.hpp"
#include "engine/renderer/Error.hpp"
#include "common/Systems.hpp"
#include "common/Components.hpp"
#include "common/Components_client.hpp"
#include "common/Components_client_sdl.hpp"
#include "common/Packets.hpp"
#include "engine/network/Udpsocket.hpp"
#include "common/Systems.hpp"
#include "Background.hpp"
#include "common/Systems_client_sdl.hpp"

R_Type::Rtype::Rtype()
: _app("R-Type", 800, 600)
{
    try
    {
        asio::io_context io;
        _client = std::make_unique<engine::net::UdpSocket>(io, 0);
        _serverEndpoint = std::make_unique<asio::ip::udp::endpoint>(asio::ip::make_address("127.0.0.1"), 4242);

        // --- STEP 1: Connect ---
        ConnectReq req{42};
        PacketHeader hdr{CONNECT_REQ, sizeof(ConnectReq), 0};
        std::vector<uint8_t> buf(sizeof(ConnectReq));
        std::memcpy(buf.data(), &req, sizeof(ConnectReq));
        _client->send(hdr, buf, *_serverEndpoint);
        std::cout << "Sent CONNECT_REQ\n";
        _registry.register_component<component::drawable>();
        _registry.register_component<component::position>();
        _registry.register_component<component::velocity>();
        _registry.register_component<component::controllable>();
        _registry.register_component<component::entity_kind>();
        _registry.register_component<component::collision_state>();
        _background = std::make_unique<Background>(*this);
        _playerTexture = std::make_unique<Player>(*this);
    }
    catch(const R_Graphic::Error& e)
    {
        throw R_Graphic::Error(e);
    }
}

void R_Type::Rtype::update(float deltaTime,
    const std::vector<R_Events::Event> &events)
    {
        for (auto &ev : events) {
            if (ev.type == R_Events::Type::KeyDown)
            {
                _keys |= keyToBit(ev.key.code);
            }
            else if (ev.type == R_Events::Type::KeyUp)
            {
                _keys &= ~keyToBit(ev.key.code);
            }
        }
        // if (R_Events::hasEvent(events, R_Events::Type::FocusGained)) {
            InputPacket inp{};
            inp.clientId = _player;
            inp.tick = _tick++;
            inp.keys = _keys;
            PacketHeader ihdr{INPUT, sizeof(InputPacket), _tick};
            std::vector<uint8_t> ibuf(sizeof(InputPacket));
            std::memcpy(ibuf.data(), &inp, sizeof(InputPacket));
            _client->send(ihdr, ibuf, *_serverEndpoint);
        // }
        receiveSnapshot();
        auto& positions = _registry.get_components<component::position>();
        auto& velocities = _registry.get_components<component::velocity>();
        auto& controls = _registry.get_components<component::controllable>();
        auto& kinds = _registry.get_components<component::entity_kind>();
        auto& collisions = _registry.get_components<component::collision_state>();
        position_system(_registry, positions, velocities, deltaTime);
        control_system(_registry, velocities, controls, _keys);
        scroll_reset_system(_registry, positions, kinds, _app);
        _registry.run_systems();

}

void R_Type::Rtype::receiveSnapshot()
{
    const uint32_t NET_BASE = 1000;

    while (auto pkt_opt = _client->receive(_sender)) {
        auto [shdr, spayload] = *pkt_opt;

        if (shdr.type == SNAPSHOT && spayload.size() >= sizeof(Snapshot)) {
            Snapshot snap{};
            std::memcpy(&snap, spayload.data(), sizeof(Snapshot));

            auto& positions  = _registry.get_components<component::position>();
            auto& velocities = _registry.get_components<component::velocity>();
            auto& drawables  = _registry.get_components<component::drawable>();
            auto& kinds      = _registry.get_components<component::entity_kind>();
            auto& collisions = _registry.get_components<component::collision_state>();

            std::unordered_set<uint32_t> newActive;

            size_t n = snap.entityCount;
            if (spayload.size() >= sizeof(Snapshot) + n * sizeof(EntityState)) {
                auto* entities = reinterpret_cast<const EntityState*>(
                    spayload.data() + sizeof(Snapshot));

                auto ensure_slot = [](auto &arr, std::size_t idx, auto &&value) {
                    if (idx >= arr.size()) {
                        arr.insert_at(idx, std::forward<decltype(value)>(value));
                    } else if (!arr[idx]) {
                        arr.insert_at(idx, std::forward<decltype(value)>(value));
                    }
                };

                for (size_t i = 0; i < n; ++i) {
                    const EntityState& es = entities[i];

                    uint32_t idLocal = NET_BASE + es.entityId;

                    if (idLocal < kinds.size() && kinds[idLocal] &&
                        kinds[idLocal].value() == component::entity_kind::decor) {
                        continue;
                    }

                    newActive.insert(idLocal);

                    ensure_slot(positions, idLocal, component::position{});
                    ensure_slot(velocities, idLocal, component::velocity{});
                    ensure_slot(kinds, idLocal, component::entity_kind{});
                    ensure_slot(collisions, idLocal, component::collision_state{});
                    kinds[idLocal] = static_cast<component::entity_kind>(es.type);
                    std::shared_ptr<R_Graphic::Texture> tex;
                    R_Graphic::textureRect rect;
                    switch (kinds[idLocal].value())
                    {
                        case component::entity_kind::projectile:
                            tex = _playerTexture->texture;
                            rect = _playerTexture->projectileRect;
                            break;
                        default:
                            tex = _playerTexture->texture;
                            rect = _playerTexture->playerRect;
                            break;
                    }
                    ensure_slot(drawables, idLocal, component::drawable{tex, rect});

                    positions[idLocal]->x = es.x;
                    positions[idLocal]->y = es.y;
                    collisions[idLocal]->collided = (es.collided != 0);
                }
            }

            if (!_activeEntities.empty()) {
                for (auto id : _activeEntities) {
                    if (newActive.find(id) == newActive.end()) {
                        if (id == _player)
                            continue;

                        if (id < kinds.size() && kinds[id] &&
                            kinds[id].value() == component::entity_kind::decor) {
                            continue;
                        }

                        if (id < positions.size()  && positions[id])  positions[id].reset();
                        if (id < velocities.size() && velocities[id]) velocities[id].reset();
                        if (id < drawables.size()  && drawables[id])  drawables[id].reset();
                        if (id < kinds.size()      && kinds[id])      kinds[id].reset();
                        if (id < collisions.size() && collisions[id]) collisions[id].reset();
                    }
                }
            }
            _activeEntities = std::move(newActive);
        }
    }
}

void R_Type::Rtype::draw()
{
    auto& positions = _registry.get_components<component::position>();
    auto& drawables = _registry.get_components<component::drawable>();

    draw_system(_registry, positions, drawables, _app.getWindow());
}

R_Graphic::App& R_Type::Rtype::getApp()
{
    return _app;
}

engine::registry &R_Type::Rtype::getRegistry()
{
    return _registry;
}

void R_Type::Rtype::waiting_connection()
{
    bool connected = false;

    while (!connected) {
        if (auto pkt_opt = _client->receive(_sender)) {
            auto [recvHdr, payload] = *pkt_opt;
            if (recvHdr.type == CONNECT_ACK &&
                payload.size() >= sizeof(ConnectAck)) {
                ConnectAck ack{};
                std::memcpy(&ack, payload.data(), sizeof(ConnectAck));
                _player = ack.playerEntityId;
                std::cout << "I am entity: " << _player << "\n";
                connected = true;

                // --- Bootstrap my entity locally ---
                auto& positions  = _registry.get_components<component::position>();
                auto& velocities = _registry.get_components<component::velocity>();
                auto& drawables  = _registry.get_components<component::drawable>();
                auto& kinds      = _registry.get_components<component::entity_kind>();
                auto& collisions = _registry.get_components<component::collision_state>();

                if (_player >= positions.size())
                    positions.insert_at(_player,
                        component::position{100.f, 100.f});

                if (_player >= velocities.size())
                    velocities.insert_at(_player, component::velocity{});

                if (_player >= drawables.size()) {
                    drawables.insert_at(_player,
                        component::drawable{_playerTexture->texture, _playerTexture->playerRect});
                }
                if (_player >= kinds.size())
                    kinds.insert_at(_player, component::entity_kind::player);

                if (_player >= collisions.size())
                    collisions.insert_at(_player, component::collision_state{});
            }
        }
    }
}

uint8_t R_Type::Rtype::keyToBit(engine::R_Events::Key key)
{
    using namespace engine::R_Events;
    switch (key) {
        case Key::Q: case Key::Left:  return 0x01;
        case Key::D: case Key::Right: return 0x02;
        case Key::Z: case Key::Up:    return 0x04;
        case Key::S: case Key::Down:  return 0x08;
        case Key::Space:              return 0x10;
        default: return 0x00;
    }
}
