#include <iostream>
#include <SDL.h>
#include <asio.hpp>
#include "Rtype.hpp"
#include "engine/renderer/Vectors.hpp"
#include "engine/renderer/Error.hpp"
#include "common/Systems.hpp"
#include "common/Components.hpp"
#include "common/Components_client.hpp"
#include "common/Components_client_sfml.hpp"
#include "common/Packets.hpp"
#include "engine/network/Udpsocket.hpp"
#include "common/Systems.hpp"
#include "common/Systems_client_sfml.hpp"

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
        _registry.register_component<component::background_tag>();
        _registry.register_component<component::collision_state>();
        _background = std::make_unique<Background>(*this);
    }
    catch(const R_Graphic::Error& e)
    {
        throw R_Graphic::Error(e);
    }
}

void R_Type::Rtype::update(float deltaTime,
    const std::vector<R_Events::Event> &events)
    {
        
        if (R_Events::hasEvent(events, R_Events::Type::FocusGained)) {
            InputPacket inp{};
            inp.clientId = _player;
            inp.tick = tick++;
            inp.keys = keys;
            
            PacketHeader ihdr{INPUT, sizeof(InputPacket), tick};
            std::vector<uint8_t> ibuf(sizeof(InputPacket));
            std::memcpy(ibuf.data(), &inp, sizeof(InputPacket));
            _client->send(ihdr, ibuf, *_serverEndpoint);
        }
        std::cout << "here" << std::endl;
        receiveSnapshot();
        std::cout << "here2" << std::endl;
        std::cout << "here3" << std::endl;
        auto& positions = _registry.get_components<component::position>();
        auto& controls = _registry.get_components<component::controllable>();
        auto& velocities = _registry.get_components<component::velocity>();
        auto& background_tags = _registry.get_components<component::background_tag>();
        auto& kinds = _registry.get_components<component::entity_kind>();
        auto& collisions = _registry.get_components<component::collision_state>();
        
        position_system(_registry, positions, velocities, deltaTime);
        scroll_reset_system(_registry, positions, background_tags, _app);
        _registry.run_systems();
    std::cout << "here4" << std::endl;
    std::cout << "positions.size()=" << positions.size()
          << ", backgrounds.size()=" << background_tags.size() << std::endl;

}

void R_Type::Rtype::receiveSnapshot()
{
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

                // Build new active set for this snapshot
                std::unordered_set<uint32_t> newActive;

                size_t n = snap.entityCount;
                if (spayload.size() >= sizeof(Snapshot) + n * sizeof(EntityState)) {
                    auto* entities = reinterpret_cast<const EntityState*>(
                        spayload.data() + sizeof(Snapshot));

                    for (size_t i = 0; i < n; ++i) {
                        const EntityState& es = entities[i];
                        newActive.insert(es.entityId);

                        auto ensure_slot = [](auto &arr, std::size_t idx, auto &&value) {
                            if (idx >= arr.size()) {
                                arr.insert_at(idx, std::forward<decltype(value)>(value));
                            } else if (!arr[idx]) {
                                arr.insert_at(idx, std::forward<decltype(value)>(value));
                            }
                        };

                        ensure_slot(positions, es.entityId, component::position{});
                        ensure_slot(velocities, es.entityId, component::velocity{});
                        ensure_slot(kinds, es.entityId, component::entity_kind{});
                        ensure_slot(collisions, es.entityId, component::collision_state{});

                        // Apply updates
                        positions[es.entityId]->x = es.x;
                        positions[es.entityId]->y = es.y;
                        // Optionally use velocity (prediction not yet implemented)
                        // velocities[es.entityId]->vx = es.vx; velocities[es.entityId]->vy = es.vy;
                        kinds[es.entityId] = static_cast<component::entity_kind>(es.type);
                        collisions[es.entityId]->collided = (es.collided != 0);
                    }
                }

                // Remove entities that disappeared (present previously but not now)
                if (!_activeEntities.empty()) {
                    for (auto id : _activeEntities) {
 if (newActive.find(id) == newActive.end()) {
    // Ne pas supprimer les backgrounds
    if (!(kinds.size() > id && kinds[id] && *kinds[id] == component::entity_kind::background)) {
        if (id < positions.size() && positions[id]) positions[id].reset();
        if (id < velocities.size() && velocities[id]) velocities[id].reset();
        if (id < drawables.size() && drawables[id]) drawables[id].reset();
        if (id < kinds.size() && kinds[id]) kinds[id].reset();
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
                    auto tex = std::make_shared<R_Graphic::Texture>(
                        _app.getWindow(),
                        "./Assets/sprites/r-typesheet1.gif",
                        R_Graphic::doubleVec2(0, 0),
                        R_Graphic::intVec2(132, 72)
                    );
                    R_Graphic::textureRect rect(100, 0, 34, 20);
                    drawables.insert_at(_player,
                        component::drawable{tex, rect});
                }

                if (_player >= kinds.size())
                    kinds.insert_at(_player, component::entity_kind::player);

                if (_player >= collisions.size())
                    collisions.insert_at(_player, component::collision_state{});
            }
        }
    }
}


