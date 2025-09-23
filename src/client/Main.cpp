#include <asio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <chrono>

#include "engine/network/Udpsocket.hpp"
#include "engine/ecs/Registry.hpp"
#include "common/Components.hpp"
#include "common/Packets.hpp"
#include "engine/ecs/iterator/Indexed_zipper.hpp"

#include "common/Systems.hpp"

using namespace engine::net;

int main() {
    asio::io_context io;
    UdpSocket client(io, 0);

    asio::ip::udp::endpoint serverEndpoint(
        asio::ip::make_address("127.0.0.1"), 4242);

    // --- STEP 1: Connect ---
    ConnectReq req{42}; // clientId
    PacketHeader hdr{CONNECT_REQ, sizeof(ConnectReq), 0};
    std::vector<uint8_t> buf(sizeof(ConnectReq));
    std::memcpy(buf.data(), &req, sizeof(ConnectReq));
    client.send(hdr, buf, serverEndpoint);
    std::cout << "Sent CONNECT_REQ\n";

    asio::ip::udp::endpoint sender;
    bool connected = false;
    while (!connected) {
        if (auto pkt_opt = client.receive(sender)) {
            auto [recvHdr, payload] = *pkt_opt;
            if (recvHdr.type == CONNECT_ACK && payload.size() >= sizeof(ConnectAck)) {
                ConnectAck ack{};
                std::memcpy(&ack, payload.data(), sizeof(ConnectAck));
                std::cout << "Connected to serverId=" << ack.serverId
                          << " tickRate=" << ack.tickRate << "\n";
                connected = true;
            }
        }
    }

    // --- STEP 2: Setup ECS ---
    engine::registry reg;
    reg.register_component<component::position>();
    reg.register_component<component::velocity>();
    reg.register_component<component::drawable>();

    // Setup systems
    reg.add_system<component::position, component::velocity>(position_system);

    // Spawn some local entity representation (synced later via snapshots)
    // for drawing, we’ll spawn as they arrive in snapshot.

    // --- STEP 3: Setup SFML window ---
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "R-Type Client");
    window.setFramerateLimit(60);

    uint32_t tick = 0;

    // --- STEP 4: Main Loop ---
    while (window.isOpen()) {
        // Handle window events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // --- Handle Input (ZQSD) ---
        uint8_t keys = 0;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) keys |= 0x01; // left
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) keys |= 0x02; // right
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) keys |= 0x04; // up
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) keys |= 0x08; // down
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) keys |= 0x10; // shoot

        InputPacket inp{};
        inp.clientId = 42;
        inp.tick = tick++;
        inp.keys = keys;

        PacketHeader ihdr{INPUT, sizeof(InputPacket), tick};
        std::vector<uint8_t> ibuf(sizeof(InputPacket));
        std::memcpy(ibuf.data(), &inp, sizeof(InputPacket));
        client.send(ihdr, ibuf, serverEndpoint);

        // --- Receive snapshots from server ---
        while (auto pkt_opt = client.receive(sender)) {
            auto [shdr, spayload] = *pkt_opt;
            if (shdr.type == SNAPSHOT && spayload.size() >= sizeof(Snapshot)) {
                Snapshot snap{};
                std::memcpy(&snap, spayload.data(), sizeof(Snapshot));

                auto& positions = reg.get_components<component::position>();
                auto& drawables = reg.get_components<component::drawable>();

                size_t n = snap.entityCount;
                if (spayload.size() >= sizeof(Snapshot) + n * sizeof(EntityState)) {
                    auto* entities = reinterpret_cast<const EntityState*>(
                        spayload.data() + sizeof(Snapshot));

                    for (size_t i = 0; i < n; ++i) {
                        std::cout << "entity " << i << std::endl;
                        const EntityState& es = entities[i];
                        // ensure entity exists
                        if (es.entityId >= positions.size()) {
                            auto e = reg.spawn_entity();
                            reg.add_component(e, component::position{});
                            reg.add_component(e, component::velocity{});
                            reg.add_component(e, component::drawable{{20,20}, sf::Color::Green});
                        }
                        // update position
                        if (positions[es.entityId]) {
                            positions[es.entityId]->x = static_cast<int>(es.x);
                            positions[es.entityId]->y = static_cast<int>(es.y);
                        }
                    }
                }
            }
        }

        // --- Run systems locally (like animation, interp) ---
        reg.run_systems();

        // --- Render ---
        window.clear();
        auto& positions = reg.get_components<component::position>();
        auto& drawables = reg.get_components<component::drawable>();
        draw_system(reg, positions, drawables, window);
        window.display();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}