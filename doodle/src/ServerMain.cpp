#include "engine/Engine.hpp"
#include "common/Packets.hpp"
#include <iostream>
#include <random>
#include <vector>
#include <cstring>

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
    try
    {
        unsigned short port = 4242;
        if (argc > 1)
            port = static_cast<unsigned short>(std::atoi(argv[1]));

        engine::net::IoContext io;
        engine::net::UdpSocket sock(io, port);
        engine::net::Endpoint lastSender{};

        registry reg;

        reg.register_component<component::position>();
        reg.register_component<component::velocity>();
        reg.register_component<component::hitbox>();
        reg.register_component<component::platform>();
        reg.register_component<component::health>();
        reg.register_component<component::controllable>();
        reg.register_component<component::gravity>();
        reg.register_component<component::entity_kind>();

        const float SCREEN_W = 480.0f;
        const float SCREEN_H = 800.0f;
        const float GRAVITY = 1600.0f;
        const float PLAYER_JUMP_VELOCITY = -900.0f;
        const float PLAYER_MOVE_SPEED = 280.0f;
        const float PLATFORM_W = 100.0f;
        const float PLATFORM_H = 18.0f;

        entity_t player = reg.spawn_entity();
        reg.add_component(player, component::position{240.0f, 400.0f});
        reg.add_component(player, component::velocity{0.0f, 0.0f});
        reg.add_component(player, component::hitbox{28.0f, 36.0f});
        reg.add_component(player, component::controllable{});
        reg.add_component(player, component::gravity{GRAVITY});
        reg.add_component(player, component::entity_kind::player);

        std::cout << "Server: spawned player entity id=" << static_cast<std::size_t>(player) << std::endl;

        std::vector<entity_t> platforms;
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> xDist(0.0f, SCREEN_W - PLATFORM_W);
        std::uniform_real_distribution<float> spacing(80.0f, 120.0f);

        float y = 700.0f;
        std::uniform_int_distribution<int> kindDist(0, 9);
        while (y > -600.0f)
        {
            float x = xDist(rng);
            auto e = reg.spawn_entity();
            reg.add_component(e, component::position{x, y});
            reg.add_component(e, component::hitbox{PLATFORM_W, PLATFORM_H});

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
            reg.add_component(e, component::platform{pkind});
            reg.add_component(e, component::entity_kind::decor);

            if (pkind == 1) {
                float mv = (rng() % 2 == 0) ? 40.0f : -40.0f;
                reg.add_component(e, component::velocity{mv, 0.0f});
            }

            if (pkind == 2) {
                reg.add_component(e, component::health{1});
            }
            platforms.push_back(e);
            y -= spacing(rng);
        }

        float minPlatformY = 1e9f;
        for (auto e : platforms) {
            if (auto &p = reg.get_components<component::position>()[e]; p && p.has_value()) {
                minPlatformY = std::min(minPlatformY, p.value().y);
            }
        }

        using clock = std::chrono::steady_clock;
        auto lastTime = clock::now();
        float accumulator = 0.f;
        const float fixedDt = 1.0f / 60.0f;
        uint32_t tick = 0;

        std::cout << "Doodle server listening on port " << port << std::endl;

        bool running = true;
        while (running) {
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
                if (hdr.type == INPUT_PKT && payload.size() >= sizeof(InputPacket)) {
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
                            for (uint16_t ki = 0; ki < inp.keyCount; ++ki) {
                                using engine::R_Events::Key;
                                Key code = static_cast<Key>(keys[ki]);
                                if (code == Key::Left || code == Key::Q)
                                    c.inputX = -1;
                                else if (code == Key::Right || code == Key::D)
                                    c.inputX = 1;
                                else if (code == Key::Space)
                                    c.shoot = true;
                            }
                            if (VERBOSE)
                                std::cout << "Server: applied input to player: inputX=" << c.inputX << " shoot=" << c.shoot << " (from payload keys=" << inp.keyCount << ")" << std::endl;
                        }
                    }
                }
            }

            accumulator += dt;
            while (accumulator >= fixedDt) {
                ++tick;

                auto &vels = reg.get_components<component::velocity>();
                auto &controls = reg.get_components<component::controllable>();
                if (controls[player] && controls[player].has_value() && vels[player] && vels[player].has_value()) {
                    auto &c = controls[player].value();
                    auto &v = vels[player].value();
                    v.vx = c.inputX * PLAYER_MOVE_SPEED;
                }

                auto &gravArr = reg.get_components<component::gravity>();
                for (size_t i = 0; i < vels.size(); ++i) {
                    if (i < gravArr.size() && gravArr[i] && gravArr[i].has_value() && vels[i] && vels[i].has_value()) {
                        vels[i].value().vy += gravArr[i].value().ay * fixedDt;
                    }
                }

                auto &poss = reg.get_components<component::position>();
                position_system(reg, poss, vels, fixedDt);

                if (poss[player] && poss[player].has_value()) {
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
                if (poss[player] && vels[player] && hbs[player])
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
                    std::cout << "Server: tick=" << tick << " player pos=(" << poss[player].value().x << "," << poss[player].value().y << ") vel=(" << vels[player].value().vx << "," << vels[player].value().vy << ")\n";
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
    catch (const std::exception &e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
}
