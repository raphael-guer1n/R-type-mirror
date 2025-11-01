#include "engine/Engine.hpp"
#include "common/Packets.hpp"
#include <iostream>
#include <random>
#include <vector>
#include <cstring>
#include <cmath>

using namespace engine;
#include "engine/network/IoContext.hpp"
#include "engine/network/UdpSocket.hpp"
#include "engine/network/Endpoint.hpp"
//#include <SDL.h>
#include "engine/events/Events.hpp"
#include <chrono>
#include <thread>

int main(int argc, char **argv)
{
        unsigned short port = 4242;
        if (argc > 1)
            port = static_cast<unsigned short>(std::atoi(argv[1]));

        engine::net::IoContext io;
        engine::net::UdpSocket sock(io, port);
        engine::net::Endpoint lastSender{};
        bool connected = false;
        bool gameStarted = false;

        registry reg;

        reg.register_component<component::position>();
        reg.register_component<component::velocity>();
        reg.register_component<component::hitbox>();
        reg.register_component<component::platform>();
        reg.register_component<component::health>();
        reg.register_component<component::controllable>();
        reg.register_component<component::gravity>();
        reg.register_component<component::entity_kind>();
        reg.register_component<component::projectile_tag>();

        const float SCREEN_W = 480.0f;
        const float SCREEN_H = 800.0f;
        const float GRAVITY = 1600.0f;
        const float PLAYER_JUMP_VELOCITY = -900.0f;
        const float PLAYER_MOVE_SPEED = 280.0f;
        const float PLATFORM_W = 100.0f;
        const float PLATFORM_H = 18.0f;

        // Manual start platform to ensure safe spawn
        std::vector<entity_t> platforms;
        const float START_PLATFORM_W = PLATFORM_W;
        const float START_PLATFORM_H = PLATFORM_H;
        const float START_PLATFORM_X = (SCREEN_W - START_PLATFORM_W) * 0.5f;
        const float START_PLATFORM_Y = 650.0f;

        auto startPlat = reg.spawn_entity();
        reg.add_component(startPlat, component::position{START_PLATFORM_X, START_PLATFORM_Y});
        reg.add_component(startPlat, component::hitbox{START_PLATFORM_W, START_PLATFORM_H});
        reg.add_component(startPlat, component::platform{0});
        reg.add_component(startPlat, component::entity_kind::decor);
        platforms.push_back(startPlat);

        entity_t player = reg.spawn_entity();
        reg.add_component(player, component::hitbox{28.0f, 36.0f});
        // Place player on top of start platform
        float playerSpawnX = START_PLATFORM_X + (START_PLATFORM_W * 0.5f) - 14.0f; // center horizontally over platform (hb width ~28)
        float playerSpawnY = START_PLATFORM_Y - 36.0f; // on top (hb height 36)
        reg.add_component(player, component::position{playerSpawnX, playerSpawnY});
        reg.add_component(player, component::velocity{0.0f, 0.0f});
        reg.add_component(player, component::controllable{});
        reg.add_component(player, component::gravity{GRAVITY});
        reg.add_component(player, component::entity_kind::player);

        std::cout << "Server: spawned player entity id=" << static_cast<std::size_t>(player) << std::endl;
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> xDist(0.0f, SCREEN_W - PLATFORM_W);
        std::uniform_real_distribution<float> spacing(80.0f, 120.0f);

        std::uniform_int_distribution<int> kindDist(0, 9);

        float minPlatformY = START_PLATFORM_Y;

        using clock = std::chrono::steady_clock;
        auto lastTime = clock::now();
        auto lastClientActivity = clock::now();
        const auto clientTimeout = std::chrono::seconds(5);
        float accumulator = 0.f;
        const float fixedDt = 1.0f / 60.0f;
        uint32_t tick = 0;
        uint32_t lastShootTick = 0;
        const uint32_t SHOOT_COOLDOWN_TICKS = 8; // ~133ms @60Hz

        std::cout << "Doodle server listening on port " << port << std::endl;
        std::cout << "Server: waiting for client handshake (CONNECT_REQ)" << std::endl;

        // Block here until a client connects (simple handshake)
        while (!connected) {
            engine::net::Endpoint ep;
            if (auto pkt = sock.receive(ep)) {
                auto [hdr, payload] = *pkt;
                if (hdr.type == CONNECT_REQ && payload.size() >= sizeof(ConnectReq)) {
                    connected = true;
                    lastSender = ep;
                    lastClientActivity = clock::now();
                    ConnectAck ack{1u, 60u, static_cast<uint16_t>(player)};
                    std::vector<uint8_t> buf(sizeof(ConnectAck));
                    std::memcpy(buf.data(), &ack, sizeof(ConnectAck));
                    PacketHeader ah{CONNECT_ACK, static_cast<uint16_t>(buf.size()), 0};
                    try { sock.send(ah, buf, lastSender); } catch (...) {}
                    std::cout << "Server: client connected, sent CONNECT_ACK" << std::endl;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        std::cout << "Server: entering run loop" << std::endl;

        bool running = true;
        while (running) {
            // debug heartbeat
            static int heartbeat = 0;
            if ((heartbeat++ % 3000) == 0) {
                std::cout << "Server: loop heartbeat" << std::endl;
            }
            auto nowTime = clock::now();
            std::chrono::duration<float> delta = nowTime - lastTime;
            float dt = delta.count();
            lastTime = nowTime;
            if (dt > 0.05f)
                dt = 0.05f;

            engine::net::Endpoint senderEndpoint;
            if (auto pkt = sock.receive(senderEndpoint)) {
                auto [hdr, payload] = *pkt;

                constexpr bool VERBOSE = false;
                if (VERBOSE)
                    std::cout << "Server: recv packet type=" << int(hdr.type) << " size=" << hdr.size << std::endl;
                lastSender = senderEndpoint;
                if (hdr.type == CONNECT_REQ && payload.size() >= sizeof(ConnectReq)) {
                    // Mark connection established and send ACK
                    connected = true;
                    lastSender = senderEndpoint;
                    lastClientActivity = clock::now();
                    ConnectAck ack{1u, 60u, static_cast<uint16_t>(player)};
                    std::vector<uint8_t> buf(sizeof(ConnectAck));
                    std::memcpy(buf.data(), &ack, sizeof(ConnectAck));
                    PacketHeader ah{CONNECT_ACK, static_cast<uint16_t>(buf.size()), 0};
                    try { sock.send(ah, buf, lastSender); } catch (...) {}
                }
                else if (hdr.type == INPUT_PKT && payload.size() >= sizeof(InputPacket)) {
                    lastClientActivity = clock::now();
                    InputPacket inp{};
                    std::memcpy(&inp, payload.data(), sizeof(InputPacket));
                    const size_t expected = sizeof(InputPacket) + static_cast<size_t>(inp.keyCount) * sizeof(int32_t);
                    if (payload.size() >= expected) {
                        const int32_t *keys = reinterpret_cast<const int32_t *>(payload.data() + sizeof(InputPacket));
                        auto &controls = reg.get_components<component::controllable>();
                        if (controls[player] && controls[player].has_value()) {
                            auto &c = controls[player].value();
                            c.inputX = 0;
                            c.inputY = 0;
                            c.shoot = false;
                            bool startPressed = false;
                            for (uint16_t ki = 0; ki < inp.keyCount; ++ki) {
                                using engine::R_Events::Key;
                                Key code = static_cast<Key>(keys[ki]);
                                if (code == Key::Left || code == Key::Q)
                                    c.inputX = -1;
                                else if (code == Key::Right || code == Key::D)
                                    c.inputX = 1;
                                else if (code == Key::Space)
                                    c.shoot = true;
                                else if (code == Key::Enter)
                                    startPressed = true;
                            }
                            if (!gameStarted && startPressed) {
                                gameStarted = true;
                                std::cout << "Server: game started by client ENTER" << std::endl;
                            }
                            if (VERBOSE)
                                std::cout << "Server: applied input to player: inputX=" << c.inputX << " shoot=" << c.shoot << " (from payload keys=" << inp.keyCount << ")" << std::endl;
                        }
                    }
                }
            }

            // Drop connection on inactivity, but keep server running and wait for a new handshake
            if (connected) {
                auto nowC = clock::now();
                if (nowC - lastClientActivity > clientTimeout) {
                    connected = false;
                    gameStarted = false;
                    lastSender = {};
                    std::cout << "Server: client timed out, waiting for new handshake" << std::endl;
                }
            }

            accumulator += dt;
            while (accumulator >= fixedDt) {
                ++tick;

                auto &vels = reg.get_components<component::velocity>();
                auto &controls = reg.get_components<component::controllable>();
                if (gameStarted) {
                    if (controls[player] && controls[player].has_value() && vels[player] && vels[player].has_value()) {
                    auto &c = controls[player].value();
                    auto &v = vels[player].value();
                    v.vx = c.inputX * PLAYER_MOVE_SPEED;

                    // Shooting: spawn a projectile when Space is pressed with a small cooldown
                    if (c.shoot && (tick - lastShootTick >= SHOOT_COOLDOWN_TICKS)) {
                        lastShootTick = tick;
                        auto &possArr = reg.get_components<component::position>();
                        auto &hbsArr = reg.get_components<component::hitbox>();
                        if (possArr[player] && possArr[player].has_value() && hbsArr[player] && hbsArr[player].has_value()) {
                            const auto &pp = possArr[player].value();
                            const auto &phb = hbsArr[player].value();

                            // Spawn projectile slightly above the player's top center
                            const float projW = 6.0f;
                            const float projH = 12.0f;
                            float spawnX = pp.x + phb.offset_x + (phb.width * 0.5f) - (projW * 0.5f);
                            float spawnY = pp.y - projH - 2.0f;

                            // Spawn at the player's top without artificial clamp to avoid altitude locking

                            entity_t proj = reg.spawn_entity();
                            reg.add_component(proj, component::position{spawnX, spawnY});
                            // initial velocity: shoot straight up
                            reg.add_component(proj, component::velocity{0.f, -900.f});
                            reg.add_component(proj, component::hitbox{projW, projH});
                            reg.add_component(proj, component::projectile_tag{static_cast<std::uint32_t>(player), 120u, 0.f, -1.f, 900.f, 1});
                            reg.add_component(proj, component::entity_kind::playerProjectile);
                        }
                    }
                }

                }

                if (gameStarted) {
                    auto &gravArr = reg.get_components<component::gravity>();
                    for (size_t i = 0; i < vels.size(); ++i) {
                        if (i < gravArr.size() && gravArr[i] && gravArr[i].has_value() && vels[i] && vels[i].has_value()) {
                            vels[i].value().vy += gravArr[i].value().ay * fixedDt;
                        }
                    }
                }

                // Update projectile lifetimes and simple cleanup when leaving bounds
                auto &projArr = reg.get_components<component::projectile_tag>();
                auto &posArrForProj = reg.get_components<component::position>();
                std::vector<entity_t> toKillProj;
                for (size_t i = 0; i < projArr.size(); ++i) {
                    if (!projArr[i] || !projArr[i].has_value()) continue;
                    auto &pt = projArr[i].value();
                    if (pt.lifetime > 0) {
                        --pt.lifetime;
                    }
                    if (pt.lifetime == 0) {
                        toKillProj.push_back(reg.entity_from_index(i));
                        continue;
                    }
                    if (i < posArrForProj.size() && posArrForProj[i] && posArrForProj[i].has_value()) {
                        // Culling relative to player Y to avoid altitude locking at extreme coordinates
                            if (player < posArrForProj.size() && posArrForProj[player] && posArrForProj[player].has_value()) {
                            float pY = posArrForProj[player].value().y;
                            const float TOP_MARGIN = 1500.0f;     // how far above player bullets can travel
                            const float BOTTOM_MARGIN = 2500.0f;  // how far below player until cleanup
                            float y = posArrForProj[i].value().y;
                            if (y < pY - TOP_MARGIN || y > pY + BOTTOM_MARGIN) {
                                toKillProj.push_back(reg.entity_from_index(i));
                            }
                        } else {
                            // Fallback absolute culling if player position is unavailable
                            if (posArrForProj[i].value().y < -10000.0f || posArrForProj[i].value().y > 10000.0f) {
                                toKillProj.push_back(reg.entity_from_index(i));
                            }
                        }
                    }
                }
                for (auto e : toKillProj) reg.kill_entity(e);

                auto &poss = reg.get_components<component::position>();
                if (gameStarted) {
                    position_system(reg, poss, vels, fixedDt);

                    // Horizontal wrap for player (Doodle Jump style)
                    auto &hbsAll = reg.get_components<component::hitbox>();
                    if (player < poss.size() && poss[player] && poss[player].has_value() &&
                        player < hbsAll.size() && hbsAll[player] && hbsAll[player].has_value()) {
                        auto &pp = poss[player].value();
                        const auto &phb = hbsAll[player].value();
                        float left = pp.x + phb.offset_x;
                        float right = left + phb.width;
                        if (left > SCREEN_W) {
                            pp.x = -phb.width - phb.offset_x;
                        } else if (right < 0.f) {
                            pp.x = SCREEN_W - phb.offset_x;
                        }
                    }

                    // Moving platforms (kind==1) bounce within screen bounds
                    auto &platsAll = reg.get_components<component::platform>();
                    auto &velsAll = reg.get_components<component::velocity>();
                    auto &hbs = reg.get_components<component::hitbox>();
                    for (auto e : platforms) {
                        if (e >= platsAll.size() || !platsAll[e] || !platsAll[e].has_value())
                            continue;
                        if (platsAll[e].value().kind != 1)
                            continue;
                        if (e >= poss.size() || !poss[e] || !poss[e].has_value())
                            continue;
                        if (e >= velsAll.size() || !velsAll[e] || !velsAll[e].has_value())
                            continue;
                        if (e >= hbs.size() || !hbs[e] || !hbs[e].has_value())
                            continue;
                        auto &p = poss[e].value();
                        auto &v = velsAll[e].value();
                        const auto &hb = hbs[e].value();
                        float left = p.x + hb.offset_x;
                        float right = left + hb.width;
                        if (left < 0.f) {
                            p.x = -hb.offset_x;
                            v.vx = std::abs(v.vx);
                        } else if (right > SCREEN_W) {
                            p.x = SCREEN_W - hb.width - hb.offset_x;
                            v.vx = -std::abs(v.vx);
                        }
                    }
                }

                if (gameStarted && poss[player] && poss[player].has_value()) {
                    float playerY = poss[player].value().y;
                    const float SPAWN_AHEAD = 300.0f;

                    while (playerY < minPlatformY + SPAWN_AHEAD) {
                        float nx = xDist(rng);
                        float gap = spacing(rng);
                        minPlatformY -= gap;
                        auto pe = reg.spawn_entity();
                        reg.add_component(pe, component::position{nx, minPlatformY});
                        reg.add_component(pe, component::hitbox{PLATFORM_W, PLATFORM_H});
                        int kd = kindDist(rng);
                        uint8_t pkind = 0;
                        if (kd <= 5)
                            pkind = 0;
                        else if
                            (kd <= 7) pkind = 1;
                        else if
                            (kd == 8) pkind = 2;
                        else
                            pkind = 3;
                        reg.add_component(pe, component::platform{pkind});
                        reg.add_component(pe, component::entity_kind::decor);
                        if (pkind == 1) {
                            float mv = (rng() % 2 == 0) ? 40.0f : -40.0f;
                            reg.add_component(pe, component::velocity{mv, 0.0f});
                        }
                        if (pkind == 2) {
                            reg.add_component(pe, component::health{1});
                        }
                        platforms.push_back(pe);
                    }

                    const float PRUNE_BELOW = 1200.0f;
                    std::vector<entity_t> kept;
                    kept.reserve(platforms.size());
                    auto &posArr = reg.get_components<component::position>();
                    for (auto e : platforms) {
                        if (e >= posArr.size() || !posArr[e] || !posArr[e].has_value())
                            continue;
                        float py = posArr[e].value().y;
                        if (py > poss[player].value().y + PRUNE_BELOW) {
                            reg.kill_entity(e);
                        }
                        else {
                            kept.push_back(e);
                        }
                    }
                    platforms.swap(kept);

                    const size_t MAX_PLATFORMS = 800;
                    if (platforms.size() > MAX_PLATFORMS)
                    {
                        size_t removeCount = platforms.size() - MAX_PLATFORMS;
                        for (size_t i = 0; i < removeCount; ++i)
                        {
                            reg.kill_entity(platforms[i]);
                        }
                        platforms.erase(platforms.begin(), platforms.begin() + removeCount);
                    }
                }

                auto &hbs = reg.get_components<component::hitbox>();
                if (gameStarted && poss[player] && vels[player] && hbs[player])
                {
                    float prevY = poss[player].value().y - vels[player].value().vy * fixedDt;
                    float prevBottom = prevY + hbs[player].value().height;
                    float curBottom = poss[player].value().y + hbs[player].value().height;
                    float playerLeft = poss[player].value().x + hbs[player].value().offset_x;
                    float playerRight = playerLeft + hbs[player].value().width;
                    for (auto e : platforms)
                    {
                        if (!(poss[e] && hbs[e]))
                            continue;

                        float platTop = poss[e].value().y;
                        if (!(platTop >= prevBottom - 50.0f && platTop <= curBottom + 200.0f))
                            continue;

                        float platLeft = poss[e].value().x + hbs[e].value().offset_x;
                        float platRight = platLeft + hbs[e].value().width;

                        bool overlapX = (playerRight > platLeft + 1.0f) && (playerLeft < platRight - 1.0f);

                        if (vels[player].value().vy > 0 && overlapX && prevBottom <= platTop + 5.0f && curBottom > platTop) {
                            poss[player].value().y = platTop - hbs[player].value().height;

                            uint8_t pkind = 0;
                            auto &plats = reg.get_components<component::platform>();
                            if (e < plats.size() && plats[e] && plats[e].has_value()) pkind = plats[e].value().kind;
                            if (pkind == 0) {
                                vels[player].value().vy = PLAYER_JUMP_VELOCITY;
                            } else if (pkind == 1) {
                                vels[player].value().vy = PLAYER_JUMP_VELOCITY;
                                if (e < vels.size() && vels[e] && vels[e].has_value()) {
                                    vels[player].value().vx += vels[e].value().vx * 0.5f;
                                }
                            } else if (pkind == 2) {
                                vels[player].value().vy = PLAYER_JUMP_VELOCITY;
                                reg.kill_entity(e);
                            } else if (pkind == 3) {
                                vels[player].value().vy = PLAYER_JUMP_VELOCITY * 1.5f;
                            } else {
                                vels[player].value().vy = PLAYER_JUMP_VELOCITY;
                            }
                            break;
                        }
                    }
                }

                accumulator -= fixedDt;
            }

            if (tick % 30 == 0) {
                auto &poss = reg.get_components<component::position>();
                auto &vels = reg.get_components<component::velocity>();
                if (poss[player] && poss[player].has_value() && vels[player] && vels[player].has_value()) {
                        std::cout << "Server: tick=" << tick << (gameStarted ? " [RUN]" : " [WAIT]") << " player pos=(" << poss[player].value().x << "," << poss[player].value().y << ") vel=(" << vels[player].value().vx << "," << vels[player].value().vy << ")\n";
                }
            }

            std::vector<EntityState> states;
            auto &positions = reg.get_components<component::position>();
            auto &velocities = reg.get_components<component::velocity>();
            auto &kinds = reg.get_components<component::entity_kind>();
            auto &hitboxes = reg.get_components<component::hitbox>();

            for (size_t i = 0; i < positions.size(); ++i) {
                if (!positions[i] || !positions[i].has_value())
                    continue;
                EntityState st{};
                st.entityId = static_cast<uint32_t>(i);
                st.x = positions[i].value().x;
                st.y = positions[i].value().y;
                if (i < velocities.size() && velocities[i] && velocities[i].has_value()) {
                    st.vx = velocities[i].value().vx;
                    st.vy = velocities[i].value().vy;
                } else {
                    st.vx = st.vy = 0.f;
                }
                st.type = (i < kinds.size() && kinds[i] && kinds[i].has_value()) ? static_cast<uint8_t>(kinds[i].value()) : static_cast<uint8_t>(component::entity_kind::unknown);
                st.hp = 0;
                st.collided = false;

                auto &plats = reg.get_components<component::platform>();
                st.platformType = (i < plats.size() && plats[i] && plats[i].has_value()) ? plats[i].value().kind : 0;
                if (i < hitboxes.size() && hitboxes[i] && hitboxes[i].has_value()) {
                    st.hb_w = hitboxes[i].value().width;
                    st.hb_h = hitboxes[i].value().height;
                    st.hb_ox = hitboxes[i].value().offset_x;
                    st.hb_oy = hitboxes[i].value().offset_y;
                }
                states.push_back(st);
            }

            Snapshot snap{tick, static_cast<uint16_t>(states.size())};
            std::vector<uint8_t> payload(sizeof(Snapshot) + states.size() * sizeof(EntityState));
            std::memcpy(payload.data(), &snap, sizeof(Snapshot));
            std::memcpy(payload.data() + sizeof(Snapshot), states.data(), states.size() * sizeof(EntityState));
            PacketHeader hdr{SNAPSHOT, static_cast<uint16_t>(payload.size()), tick};

            if (!lastSender.address.empty() && lastSender.port != 0) {
                try {
                    sock.send(hdr, payload, lastSender);
                } catch (const std::exception &ex) {
                    (void)ex;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }

    return 0;
}

