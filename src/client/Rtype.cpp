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

        asio::ip::udp::endpoint serverEndpoint(
            asio::ip::make_address("127.0.0.1"), 4242);

        // --- STEP 1: Connect ---
        ConnectReq req{42};
        PacketHeader hdr{CONNECT_REQ, sizeof(ConnectReq), 0};
        std::vector<uint8_t> buf(sizeof(ConnectReq));
        std::memcpy(buf.data(), &req, sizeof(ConnectReq));
        _client->send(hdr, buf, serverEndpoint);
        std::cout << "Sent CONNECT_REQ\n";
        _registry.register_component<component::drawable>();
        _registry.register_component<component::position>();
        _registry.register_component<component::velocity>();
        _registry.register_component<component::controllable>();
        _registry.register_component<component::entity_kind>();
        _registry.register_component<component::collision_state>();
        _background = std::make_unique<Background>(*this);
        std::cout << "here" << std::endl;
    }
    catch(const R_Graphic::Error& e)
    {
        throw R_Graphic::Error(e);
    }
}

void R_Type::Rtype::update(float deltaTime,
    const std::vector<R_Events::Event> &events)
{
    // if (R_Events::hasEvent(events, R_Events::Type::FocusGained)) {
    //     InputPacket inp{};
    //     // inp.clientId = 
    // }
    _registry.run_systems();
    auto& positions = _registry.get_components<component::position>();
    auto& controls = _registry.get_components<component::controllable>();
    auto& velocities = _registry.get_components<component::velocity>();
    auto& background_tags = _registry.get_components<component::background_tag>();

    position_system(_registry, positions, velocities, deltaTime);
    scroll_reset_system(_registry, positions, background_tags, _app);
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
    uint32_t myEntity = 0;
    asio::ip::udp::endpoint sender;
    bool connected = false;

    while (!connected) {
        if (auto pkt_opt = _client->receive(sender)) {
            auto [recvHdr, payload] = *pkt_opt;
            if (recvHdr.type == CONNECT_ACK &&
                payload.size() >= sizeof(ConnectAck)) {
                ConnectAck ack{};
                std::memcpy(&ack, payload.data(), sizeof(ConnectAck));
                myEntity = ack.playerEntityId;
                std::cout << "I am entity: " << myEntity << "\n";
                connected = true;

                // --- Bootstrap my entity locally ---
                auto& positions  = _registry.get_components<component::position>();
                auto& velocities = _registry.get_components<component::velocity>();
                auto& drawables  = _registry.get_components<component::drawable>();
                auto& kinds      = _registry.get_components<component::entity_kind>();
                auto& collisions = _registry.get_components<component::collision_state>();

                if (myEntity >= positions.size())
                    positions.insert_at(myEntity,
                                        component::position{100.f, 100.f});

                if (myEntity >= velocities.size())
                    velocities.insert_at(myEntity, component::velocity{});

                if (myEntity >= drawables.size()) {
                    auto tex = std::make_shared<R_Graphic::Texture>(
                        _app.getWindow(),
                        "./Assets/sprites/r-typesheet1.gif",
                        R_Graphic::doubleVec2(0, 0),
                        R_Graphic::intVec2(132, 72)
                    );
                    R_Graphic::textureRect rect(100, 0, 34, 20);
                    drawables.insert_at(myEntity,
                        component::drawable{tex, rect});
                }

                if (myEntity >= kinds.size())
                    kinds.insert_at(myEntity, component::entity_kind::player);

                if (myEntity >= collisions.size())
                    collisions.insert_at(myEntity, component::collision_state{});
            }
        }
    }
}

