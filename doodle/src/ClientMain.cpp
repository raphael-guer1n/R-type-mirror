#include "engine/Engine.hpp"
#include "common/Packets.hpp"
#include <iostream>
#include <unordered_set>
#include <vector>
#include <cstring>

#include "engine/network/IoContext.hpp"
#include "engine/network/UdpSocket.hpp"
#include "engine/network/Endpoint.hpp"

using namespace engine;
using namespace engine::R_Graphic;

int main(int argc, char **argv) {
    try {
        std::string serverIp = "127.0.0.1";
        unsigned short serverPort = 4242;
        if (argc > 1)
            serverIp = argv[1];
        if (argc > 2)
            serverPort = static_cast<unsigned short>(std::atoi(argv[2]));

        Window window("Doodle - Client", 480, 800);
        Renderer renderer(window);

    engine::net::IoContext io;
    engine::net::UdpSocket sock(io, 0);
    engine::net::Endpoint serverEndpoint = engine::net::make_endpoint(serverIp, serverPort);
    bool connected = false;
    uint32_t lastConnReqMs = SDL_GetTicks();

        std::cout << "Client: using server " << serverIp << ":" << serverPort << std::endl;

        registry reg;
        reg.register_component<component::position>();
        reg.register_component<component::velocity>();
        reg.register_component<component::hitbox>();
        reg.register_component<component::platform>();
        reg.register_component<component::entity_kind>();

        bool running = true;
        Uint64 lastCounter = SDL_GetPerformanceCounter();
        double perfFreq = static_cast<double>(SDL_GetPerformanceFrequency());
        uint32_t tick = 0;

        std::unordered_set<int32_t> pressed;
        std::unordered_set<int32_t> prevPressed;

        constexpr bool VERBOSE = true;
        const uint32_t INPUT_SEND_HZ = 20;
        const uint32_t INPUT_SEND_MS = 1000u / INPUT_SEND_HZ;
        uint32_t lastInputSendMs = SDL_GetTicks();
        uint32_t lastParseLogMs = SDL_GetTicks();
        float cameraYPrev = 0.0f;

        while (running) {
            auto events = window.pollEvents(running);
            for (auto &ev : events) {
                if (ev.type == engine::R_Events::Type::Quit) {
                    running = false; break;
                }
                if (ev.type == engine::R_Events::Type::KeyDown) {
                    pressed.insert(static_cast<int32_t>(ev.key.code));
                }
                if (ev.type == engine::R_Events::Type::KeyUp) {
                    pressed.erase(static_cast<int32_t>(ev.key.code));
                }
            }

            if (pressed != prevPressed) {
                std::cout << "Client: pressed keys:";
                for (auto k : pressed)
                    std::cout << ' ' << k;
                std::cout << "\n";
                prevPressed = pressed;
            }

            uint32_t nowMs = SDL_GetTicks();
            if (!connected) {
                if (nowMs - lastConnReqMs >= 1000u) {
                    lastConnReqMs = nowMs;
                    ConnectReq req{0u};
                    std::vector<uint8_t> buf(sizeof(ConnectReq));
                    std::memcpy(buf.data(), &req, sizeof(ConnectReq));
                    PacketHeader h{CONNECT_REQ, static_cast<uint16_t>(buf.size()), 0};
                    try { sock.send(h, buf, serverEndpoint); if (VERBOSE) std::cout << "Client: sent CONNECT_REQ" << std::endl; } catch (...) {}
                }
            } else {
                if (nowMs - lastInputSendMs >= INPUT_SEND_MS) {
                    lastInputSendMs = nowMs;
                    InputPacket inp{};
                    inp.clientId = 0;
                    inp.tick = tick++;
                    inp.keyCount = static_cast<uint16_t>(pressed.size());
                    std::vector<int32_t> keys;
                    keys.reserve(pressed.size());
                    for (auto k : pressed)
                        keys.push_back(static_cast<int32_t>(k));
                    const uint16_t payloadSize = static_cast<uint16_t>(sizeof(InputPacket) + keys.size() * sizeof(int32_t));
                    std::vector<uint8_t> ibuf(payloadSize);
                    std::memcpy(ibuf.data(), &inp, sizeof(InputPacket));
                    if (!keys.empty())
                        std::memcpy(ibuf.data() + sizeof(InputPacket), keys.data(), keys.size() * sizeof(int32_t));
                    PacketHeader outh{INPUT_PKT, static_cast<uint16_t>(ibuf.size()), inp.tick};
                    try {
                        sock.send(outh, ibuf, serverEndpoint);
                        if (VERBOSE)
                            std::cout << "Client: sent input keyCount=" << inp.keyCount << " tick=" << inp.tick << std::endl;
                    } catch (const std::exception &ex) {
                        if (VERBOSE)
                            std::cerr << "Client: send error: " << ex.what() << std::endl;
                    }
                }
            }

            engine::net::Endpoint senderEndpoint;
            while (true) {
                auto pkt_opt = sock.receive(senderEndpoint);
                if (!pkt_opt)
                    break;
                auto [hdr, payload] = *pkt_opt;
                if (VERBOSE)
                    std::cout << "Client: received packet type=" << int(hdr.type) << " size=" << hdr.size << std::endl;
                if (hdr.type == CONNECT_ACK && payload.size() >= sizeof(ConnectAck)) {
                    ConnectAck ack{};
                    std::memcpy(&ack, payload.data(), sizeof(ConnectAck));
                    connected = true;
                    if (VERBOSE) std::cout << "Client: CONNECT_ACK received tickRate=" << ack.tickRate << " playerId=" << ack.playerEntityId << std::endl;
                }
                else if (hdr.type == SNAPSHOT && payload.size() >= sizeof(Snapshot)) {
                    Snapshot snap{};
                    std::memcpy(&snap, payload.data(), sizeof(Snapshot));
                    if (VERBOSE)
                        std::cout << "Client: received SNAPSHOT tick=" << snap.tick << " entityCount=" << snap.entityCount << " hdr.seq=" << hdr.seq << " payload=" << payload.size() << std::endl;

                    size_t n = snap.entityCount;
                    size_t payloadEntitiesBytes = payload.size() - sizeof(Snapshot);

                    size_t countWithPlatform = (payloadEntitiesBytes) / sizeof(EntityState);

                    int playerCount = 0;
                    auto &positions = reg.get_components<component::position>();
                    auto &velocities = reg.get_components<component::velocity>();
                    auto &hitboxes = reg.get_components<component::hitbox>();
                    auto &platforms = reg.get_components<component::platform>();
                    auto &kinds = reg.get_components<component::entity_kind>();

                    if (countWithPlatform >= n) {
                        // Track which entity IDs are present in this snapshot to prune stale client-side entities
                        std::vector<uint8_t> present;
                        const EntityState *states = reinterpret_cast<const EntityState *>(payload.data() + sizeof(Snapshot));
                        for (size_t i = 0; i < n; ++i) {
                            const EntityState &es = states[i];
                            size_t idx = es.entityId;
                            if (idx >= present.size()) present.resize(idx + 1, 0);
                            present[idx] = 1;
                            if (idx >= positions.size())
                                positions.insert_at(idx, component::position{});
                            if (idx >= velocities.size())
                                velocities.insert_at(idx, component::velocity{});
                            if (idx >= hitboxes.size())
                                hitboxes.insert_at(idx, component::hitbox{});
                            if (idx >= platforms.size())
                                platforms.insert_at(idx, component::platform{});
                            if (idx >= kinds.size())
                                kinds.insert_at(idx, component::entity_kind{});
                            positions[idx] = component::position{es.x, es.y};
                            velocities[idx] = component::velocity{es.vx, es.vy};
                            hitboxes[idx] = component::hitbox{es.hb_w, es.hb_h, es.hb_ox, es.hb_oy};
                            kinds[idx] = static_cast<component::entity_kind>(es.type);
                            platforms[idx] = component::platform{es.platformType};
                            if (kinds[idx] == component::entity_kind::player)
                                ++playerCount;
                        }

                        // Prune any local entities not present in this snapshot
                        size_t maxSize = positions.size();
                        for (size_t i = 0; i < maxSize; ++i) {
                            bool isPresent = (i < present.size()) ? (present[i] != 0) : false;
                            if (!isPresent) {
                                if (i < positions.size()) positions.erase(i);
                                if (i < velocities.size()) velocities.erase(i);
                                if (i < hitboxes.size()) hitboxes.erase(i);
                                if (i < platforms.size()) platforms.erase(i);
                                if (i < kinds.size()) kinds.erase(i);
                            }
                        }
                    } else {
                        size_t usable = std::min(n, countWithPlatform);
                        if (usable > 0) {
                            std::vector<uint8_t> present;
                            const EntityState *states = reinterpret_cast<const EntityState *>(payload.data() + sizeof(Snapshot));
                            for (size_t i = 0; i < usable; ++i) {
                                const EntityState &es = states[i];
                                size_t idx = es.entityId;
                                if (idx >= present.size()) present.resize(idx + 1, 0);
                                present[idx] = 1;
                                if (idx >= positions.size())
                                    positions.insert_at(idx, component::position{});
                                if (idx >= velocities.size())
                                    velocities.insert_at(idx, component::velocity{});
                                if (idx >= hitboxes.size())
                                    hitboxes.insert_at(idx, component::hitbox{});
                                if (idx >= platforms.size())
                                    platforms.insert_at(idx, component::platform{});
                                if (idx >= kinds.size())
                                    kinds.insert_at(idx, component::entity_kind{});
                                positions[idx] = component::position{es.x, es.y};
                                velocities[idx] = component::velocity{es.vx, es.vy};
                                hitboxes[idx] = component::hitbox{es.hb_w, es.hb_h, es.hb_ox, es.hb_oy};
                                kinds[idx] = static_cast<component::entity_kind>(es.type);
                                platforms[idx] = component::platform{es.platformType};
                                if (kinds[idx] == component::entity_kind::player)
                                    ++playerCount;
                            }

                            // Prune local entities not present in the subset parsed
                            size_t maxSize = positions.size();
                            for (size_t i = 0; i < maxSize; ++i) {
                                bool isPresent = (i < present.size()) ? (present[i] != 0) : false;
                                if (!isPresent) {
                                    if (i < positions.size()) positions.erase(i);
                                    if (i < velocities.size()) velocities.erase(i);
                                    if (i < hitboxes.size()) hitboxes.erase(i);
                                    if (i < platforms.size()) platforms.erase(i);
                                    if (i < kinds.size()) kinds.erase(i);
                                }
                            }
                        }
                        if (VERBOSE)
                            std::cerr << "Client: snapshot size mismatch (hdr.entityCount=" << n << ", bytes=" << payloadEntitiesBytes << ") parsed=" << usable << " entries" << std::endl;
                    }

                    uint32_t nowLog = SDL_GetTicks();
                    if (VERBOSE || nowLog - lastParseLogMs > 1000u) {
                        lastParseLogMs = nowLog;
                        std::cout << "Client: parsed snapshot tick=" << snap.tick << " entities=" << n << " playerCount=" << playerCount << " positionsSize=" << positions.size() << " kindsSize=" << kinds.size() << std::endl;
                    }
                }
            }

            renderer.clear();
            auto &positions = reg.get_components<component::position>();
            auto &hitboxes = reg.get_components<component::hitbox>();
            auto &platforms = reg.get_components<component::platform>();
            auto &kinds = reg.get_components<component::entity_kind>();
            ssize_t playerIdx = -1;
            for (size_t i = 0; i < kinds.size(); ++i) {
                if (kinds[i] && kinds[i].has_value() && kinds[i].value() == component::entity_kind::player) {
                    playerIdx = static_cast<ssize_t>(i);
                    break;
                }
            }

            const int SCREEN_W = 480;
            const int SCREEN_H = 800;
            float cameraY = cameraYPrev;
            if (playerIdx >= 0 && playerIdx < static_cast<ssize_t>(positions.size()) && positions[playerIdx]) {
                float targetY = positions[playerIdx].value().y - (SCREEN_H / 2.0f);
                const float SMOOTH = 0.12f;
                cameraY = cameraYPrev + (targetY - cameraYPrev) * SMOOTH;
            }

            for (size_t i = 0; i < positions.size(); ++i)
            {
                if (!(positions[i] && hitboxes[i]))
                    continue;
                auto p = positions[i].value();
                auto hb = hitboxes[i].value();
                int screenX = static_cast<int>(p.x);
                int screenY = static_cast<int>(p.y - cameraY);

                uint8_t r = 0, g = 0, b = 0, a = 255;
                if (i < kinds.size() && kinds[i] && kinds[i].has_value()) {
                    auto k = kinds[i].value();
                    if (k == component::entity_kind::player) {
                        r = 0;
                        g = 200;
                        b = 0;
                    }
                    else if (k == component::entity_kind::playerProjectile) {
                        r = 255; g = 60; b = 60; // player bullet
                    }
                    else if (k == component::entity_kind::decor) {
                        if (i < platforms.size() && platforms[i] && platforms[i].has_value()) {
                            uint8_t pk = platforms[i].value().kind;
                            switch (pk)
                            {
                            case 1: // moving
                                r = 50; g = 130; b = 255;
                                break;
                            case 2: // fragile
                                r = 210; g = 180; b = 0;
                                break;
                            case 3: // bounce
                                r = 0; g = 200; b = 200;
                                break;
                            default: // regular
                                r = 100; g = 100; b = 100;
                                break;
                            }
                        }
                        else {
                            r = 0; g = 0; b = 0;
                        }
                    } else {
                        r = 255;
                        g = 0;
                        b = 0;
                    }
                }
                renderer.setDrawColor(r, g, b, a);
                renderer.fillRect(screenX, screenY, static_cast<int>(hb.width), static_cast<int>(hb.height));

                if (static_cast<ssize_t>(i) == playerIdx) {
                    renderer.setDrawColor(0, 255, 0, 255);
                    renderer.drawRect(screenX - 2, screenY - 2, static_cast<int>(hb.width) + 4, static_cast<int>(hb.height) + 4);
                }
            }
            if (playerIdx == -1) {
                renderer.setDrawColor(255, 0, 0, 255);
                renderer.fillRect((SCREEN_W / 2) - 40, (SCREEN_H / 2) - 40, 80, 80);
                if (VERBOSE && SDL_GetTicks() - lastParseLogMs > 1000u) {
                    std::cerr << "Client: WARNING - no player entity found in snapshot (drawing debug rect)" << std::endl;
                }
            }
            renderer.display();
            cameraYPrev = cameraY;

            SDL_Delay(16);
        }

        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Client error: " << e.what() << std::endl;
        return 1;
    }
}
