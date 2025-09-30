#include <asio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <chrono>

#include "engine/network/Udpsocket.hpp"
#include "engine/ecs/Registry.hpp"
#include "common/Components.hpp"
#include "engine/ecs/Components_client_sfml.hpp"
#include "common/Packets.hpp"
#include "engine/ecs/iterator/Indexed_zipper.hpp"
#include "engine/ecs/Systems_client_sfml.hpp"
#include "engine/ecs/Systems.hpp"

using namespace engine::net;

int main() {
    asio::io_context io;
    UdpSocket client(io, 0);

    asio::ip::udp::endpoint serverEndpoint(
        asio::ip::make_address("127.0.0.1"), 4242);

    // --- STEP 1: Connect ---
    ConnectReq req{42};
    PacketHeader hdr{CONNECT_REQ, sizeof(ConnectReq), 0};
    std::vector<uint8_t> buf(sizeof(ConnectReq));
    std::memcpy(buf.data(), &req, sizeof(ConnectReq));
    client.send(hdr, buf, serverEndpoint);
    std::cout << "Sent CONNECT_REQ\n";

    // ECS Setup (register components now, needed for bootstrapping)
    engine::registry reg;
    reg.register_component<component::position>();
    reg.register_component<component::velocity>();
    reg.register_component<component::drawable>();
    reg.register_component<component::entity_kind>();
    reg.register_component<component::collision_state>();

    reg.add_system<component::position, component::velocity>(position_system);

    uint32_t myEntity = 0;
    asio::ip::udp::endpoint sender;
    bool connected = false;

    while (!connected) {
        if (auto pkt_opt = client.receive(sender)) {
            auto [recvHdr, payload] = *pkt_opt;
            if (recvHdr.type == CONNECT_ACK &&
                payload.size() >= sizeof(ConnectAck)) {
                ConnectAck ack{};
                std::memcpy(&ack, payload.data(), sizeof(ConnectAck));
                myEntity = ack.playerEntityId;
                std::cout << "I am entity: " << myEntity << "\n";
                connected = true;

                // --- Bootstrap my entity locally ---
                auto& positions  = reg.get_components<component::position>();
                auto& velocities = reg.get_components<component::velocity>();
                auto& drawables  = reg.get_components<component::drawable>();
                auto& kinds      = reg.get_components<component::entity_kind>();
                auto& collisions = reg.get_components<component::collision_state>();

                if (myEntity >= positions.size())
                    positions.insert_at(myEntity,
                                        component::position{100.f, 100.f});

                if (myEntity >= velocities.size())
                    velocities.insert_at(myEntity, component::velocity{});

                if (myEntity >= drawables.size())
                    drawables.insert_at(myEntity,
                        component::drawable{{40.f, 40.f}, sf::Color::Green});

                if (myEntity >= kinds.size())
                    kinds.insert_at(myEntity, component::entity_kind::player);

                if (myEntity >= collisions.size())
                    collisions.insert_at(myEntity, component::collision_state{});
            }
        }
    }

    // --- STEP 2: Window ---
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "R-Type Client");
    window.setFramerateLimit(60);

    uint32_t tick = 0;
    uint8_t keys = 0;

    // --- STEP 3: Main Loop ---
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Q
                    || event.key.code == sf::Keyboard::Left) keys |= 0x01;
                if (event.key.code == sf::Keyboard::D
                    || event.key.code == sf::Keyboard::Right) keys |= 0x02;
                if (event.key.code == sf::Keyboard::Z
                    || event.key.code == sf::Keyboard::Up) keys |= 0x04;
                if (event.key.code == sf::Keyboard::S
                    || event.key.code == sf::Keyboard::Down) keys |= 0x08;
                if (event.key.code == sf::Keyboard::Space) keys |= 0x10;
            }
            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::Q
                    || event.key.code == sf::Keyboard::Left) keys &= ~0x01;
                if (event.key.code == sf::Keyboard::D
                    || event.key.code == sf::Keyboard::Right) keys &= ~0x02;
                if (event.key.code == sf::Keyboard::Z
                    || event.key.code == sf::Keyboard::Up) keys &= ~0x04;
                if (event.key.code == sf::Keyboard::S
                    || event.key.code == sf::Keyboard::Down) keys &= ~0x08;
                if (event.key.code == sf::Keyboard::Space) keys &= ~0x10;
            }
        }

        // --- Send input each frame when focused ---
        if (window.hasFocus()) {
            InputPacket inp{};
            inp.clientId = myEntity;
            inp.tick = tick++;
            inp.keys = keys;

            PacketHeader ihdr{INPUT, sizeof(InputPacket), tick};
            std::vector<uint8_t> ibuf(sizeof(InputPacket));
            std::memcpy(ibuf.data(), &inp, sizeof(InputPacket));
            client.send(ihdr, ibuf, serverEndpoint);
        }

        // --- Receive snapshots ---
        while (auto pkt_opt = client.receive(sender)) {
            auto [shdr, spayload] = *pkt_opt;
            if (shdr.type == SNAPSHOT &&
                spayload.size() >= sizeof(Snapshot)) {
                Snapshot snap{};
                std::memcpy(&snap, spayload.data(), sizeof(Snapshot));

                auto& positions  = reg.get_components<component::position>();
                auto& velocities = reg.get_components<component::velocity>();
                auto& drawables  = reg.get_components<component::drawable>();
                auto& kinds      = reg.get_components<component::entity_kind>();
                auto& collisions = reg.get_components<component::collision_state>();

                size_t n = snap.entityCount;
                if (spayload.size() >= sizeof(Snapshot) + n * sizeof(EntityState)) {
                    auto* entities = reinterpret_cast<const EntityState*>(
                        spayload.data() + sizeof(Snapshot));

                    for (size_t i = 0; i < n; ++i) {
                        const EntityState& es = entities[i];

                        // Ensure indices exist
                        if (es.entityId >= positions.size())
                            positions.insert_at(es.entityId, component::position{});
                        if (es.entityId >= velocities.size())
                            velocities.insert_at(es.entityId, component::velocity{});
                        if (es.entityId >= drawables.size())
                            drawables.insert_at(es.entityId,
                                component::drawable{{40,40}, sf::Color::White});
                        if (es.entityId >= kinds.size())
                            kinds.insert_at(es.entityId, component::entity_kind{});
                        if (es.entityId >= collisions.size())
                            collisions.insert_at(es.entityId, component::collision_state{});

                        // Apply updates
                        if (positions[es.entityId]) {
                            positions[es.entityId]->x = es.x;
                            positions[es.entityId]->y = es.y;
                        }
                        if (kinds[es.entityId]) {
                            kinds[es.entityId] =
                                static_cast<component::entity_kind>(es.type);
                        } else {
                            kinds[es.entityId].emplace(
                                static_cast<component::entity_kind>(es.type));
                        }
                        if (collisions[es.entityId]) {
                            collisions[es.entityId]->collided = (es.collided != 0);
                        }
                    }
                }
            }
        }

        // --- Run systems ---
        reg.run_systems();

        // --- Update colors dynamically ---
        auto& drawables  = reg.get_components<component::drawable>();
        auto& kinds      = reg.get_components<component::entity_kind>();
        auto& collisions = reg.get_components<component::collision_state>();

        for (size_t i = 0; i < drawables.size(); ++i) {
            if (i >= kinds.size() || !drawables[i] || !kinds[i]) continue;

            auto& dr = drawables[i].value();

            switch (kinds[i].value()) {
                case component::entity_kind::player:
                    if (i == myEntity) {
                        dr.color = sf::Color::Green; // Me
                        if (i < collisions.size() && collisions[i] &&
                            collisions[i]->collided)
                            dr.color = sf::Color::Magenta;
                    } else {
                        dr.color = sf::Color::Yellow; // Other players
                    }
                    break;
                case component::entity_kind::enemy:
                    dr.color = sf::Color::Red;
                    break;
                case component::entity_kind::decor:
                    dr.color = sf::Color::Blue;
                    break;
                default:
                    dr.color = sf::Color::White;
                    break;
            }
        }

        // --- Render ---
        window.clear();
        auto& positions = reg.get_components<component::position>();
        draw_system(reg, positions, drawables, window);
        window.display();
    }

    return 0;
}