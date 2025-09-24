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

    uint32_t myEntity = 0;
    asio::ip::udp::endpoint sender;
    bool connected = false;
    while (!connected) {
        if (auto pkt_opt = client.receive(sender)) {
            auto [recvHdr, payload] = *pkt_opt;
            if (recvHdr.type == CONNECT_ACK && payload.size() >= sizeof(ConnectAck)) {
                ConnectAck ack{};
                std::memcpy(&ack, payload.data(), sizeof(ConnectAck));
                myEntity = ack.playerEntityId;
                std::cout << "I am entity: " << myEntity << "\n";
                connected = true;
            }
        }
    }

    // --- STEP 2: Setup ECS ---
    engine::registry reg;
    reg.register_component<component::position>();
    reg.register_component<component::velocity>();
    reg.register_component<component::drawable>();
    reg.register_component<component::entity_kind>();
    reg.register_component<component::collision_state>();

    reg.add_system<component::position, component::velocity>(position_system);

    // --- STEP 3: Window ---
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "R-Type Client");
    window.setFramerateLimit(60);

    uint32_t tick = 0;
    uint8_t keys = 0;

    // --- STEP 4: Main Loop ---
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Q) keys |= 0x01;
                if (event.key.code == sf::Keyboard::D) keys |= 0x02;
                if (event.key.code == sf::Keyboard::Z) keys |= 0x04;
                if (event.key.code == sf::Keyboard::S) keys |= 0x08;
                if (event.key.code == sf::Keyboard::Space) keys |= 0x10;
            }
            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::Q) keys &= ~0x01;
                if (event.key.code == sf::Keyboard::D) keys &= ~0x02;
                if (event.key.code == sf::Keyboard::Z) keys &= ~0x04;
                if (event.key.code == sf::Keyboard::S) keys &= ~0x08;
                if (event.key.code == sf::Keyboard::Space) keys &= ~0x10;
            }
        }

        // Send input each frame when focused
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
            if (shdr.type == SNAPSHOT && spayload.size() >= sizeof(Snapshot)) {
                Snapshot snap{};
                std::memcpy(&snap, spayload.data(), sizeof(Snapshot));

                auto& positions  = reg.get_components<component::position>();
                auto& drawables  = reg.get_components<component::drawable>();
                auto& kinds      = reg.get_components<component::entity_kind>();
                auto& collisions = reg.get_components<component::collision_state>();

                size_t n = snap.entityCount;
                if (spayload.size() >= sizeof(Snapshot) + n * sizeof(EntityState)) {
                    auto* entities = reinterpret_cast<const EntityState*>(
                        spayload.data() + sizeof(Snapshot));

                    for (size_t i = 0; i < n; ++i) {
                        const EntityState& es = entities[i];
                        if (es.entityId >= positions.size()) {
                            auto e = reg.spawn_entity();
                            reg.add_component(e, component::position{});
                            reg.add_component(e, component::velocity{});
                            reg.add_component(e, component::drawable{{40,40}, sf::Color::White});
                            reg.add_component(e, component::entity_kind{});
                            reg.add_component(e, component::collision_state{});
                        }
                        if (positions[es.entityId]) {
                            positions[es.entityId]->x = es.x;
                            positions[es.entityId]->y = es.y;
                        }
                        if (kinds[es.entityId]) {
                            kinds[es.entityId] = component::entity_kind{
                                static_cast<component::entity_kind>(es.type) };
                        }
                        if (collisions[es.entityId]) {
                            collisions[es.entityId]->collided = (es.collided != 0);
                        }
                    }
                }
            }
        }

        // run local systems if any
        reg.run_systems();

        // Color update
        auto& drawables  = reg.get_components<component::drawable>();
        auto& kinds      = reg.get_components<component::entity_kind>();
        auto& collisions = reg.get_components<component::collision_state>();

        for (size_t i=0; i<drawables.size(); ++i) {
            if (!drawables[i] || !kinds[i]) continue;
            auto &dr = drawables[i].value();

            switch (kinds[i].value()) {
                case component::entity_kind::player:
                    if (i == myEntity) {
                        dr.color = sf::Color::Green;
                        if (i < collisions.size() && collisions[i] && collisions[i]->collided)
                            dr.color = sf::Color::Magenta;
                    } else {
                        dr.color = sf::Color::Yellow;
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

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}