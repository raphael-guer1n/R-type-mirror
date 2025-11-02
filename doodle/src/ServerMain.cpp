#include "engine/Engine.hpp"
#include "common/Packets.hpp"
#include <iostream>
#include <random>
#include <vector>
#include <cstring>
#include <cmath>

#include "engine/network/IoContext.hpp"
#include "engine/network/UdpSocket.hpp"
#include "engine/network/Endpoint.hpp"
#include "engine/events/Events.hpp"
#include <chrono>
#include <thread>

using namespace engine;

namespace {
    constexpr float SCREEN_W = 480.0f;
    constexpr float SCREEN_H = 800.0f;
    constexpr float GRAVITY_C = 1600.0f;
    constexpr float PLAYER_JUMP_V = -900.0f;
    constexpr float PLAYER_MOVE_S = 280.0f;
    constexpr float PLATFORM_W_C = 100.0f;
    constexpr float PLATFORM_H_C = 18.0f;
    constexpr float ENEMY_W_C = 163.0f;
    constexpr float ENEMY_H_C = 102.0f;
    constexpr float STOMP_MULT = 1.3f;
    constexpr float PROJECTILE_SPEED = 1400.0f;

    using clock = std::chrono::steady_clock;

    inline void sendConnectAck(engine::net::UdpSocket &sock, const engine::net::Endpoint &to, uint16_t playerId)
    {
        ConnectAck ack{1u, 60u, playerId};
        std::vector<uint8_t> buf(sizeof(ConnectAck));
        std::memcpy(buf.data(), &ack, sizeof(ConnectAck));
        PacketHeader ah{CONNECT_ACK, static_cast<uint16_t>(buf.size()), 0};
        try { sock.send(ah, buf, to); } catch (...) {}
    }

    void waitForHandshake(engine::net::IoContext &io, engine::net::UdpSocket &sock, bool &connected, engine::net::Endpoint &lastSender, uint16_t playerId)
    {
        std::cout << "Server: waiting for client handshake (CONNECT_REQ)" << std::endl;
        while (!connected) {
            io.poll();
            engine::net::Endpoint ep; PacketHeader inHdr; std::vector<uint8_t> inPayload;
            while (sock.PollPacket(inHdr, inPayload, ep)) {
                if (inHdr.type == CONNECT_REQ && inPayload.size() >= sizeof(ConnectReq)) {
                    connected = true; lastSender = ep; sendConnectAck(sock, lastSender, playerId);
                    std::cout << "Server: client connected, sent CONNECT_ACK" << std::endl;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    void processIncomingPackets(engine::net::UdpSocket &sock, registry &reg, entity_t player, bool &gameStarted, engine::net::Endpoint &lastSender, clock::time_point &lastClientActivity)
    {
        engine::net::Endpoint senderEndpoint; PacketHeader inHdr; std::vector<uint8_t> inPayload;
        while (sock.PollPacket(inHdr, inPayload, senderEndpoint)) {
            constexpr bool VERBOSE = false;
            if (VERBOSE) std::cout << "Server: recv packet type=" << int(inHdr.type) << " size=" << inHdr.size << std::endl;
            lastSender = senderEndpoint;
            if (inHdr.type == CONNECT_REQ && inPayload.size() >= sizeof(ConnectReq)) {
                lastClientActivity = clock::now();
                sendConnectAck(sock, lastSender, static_cast<uint16_t>(player));
            } else if (inHdr.type == INPUT_PKT && inPayload.size() >= sizeof(InputPacket)) {
                lastClientActivity = clock::now();
                InputPacket inp{}; std::memcpy(&inp, inPayload.data(), sizeof(InputPacket));
                const size_t expected = sizeof(InputPacket) + static_cast<size_t>(inp.keyCount) * sizeof(int32_t);
                if (inPayload.size() >= expected) {
                    const int32_t *keys = reinterpret_cast<const int32_t *>(inPayload.data() + sizeof(InputPacket));
                    auto &controls = reg.get_components<component::controllable>();
                    if (player < controls.size() && controls[player] && controls[player].has_value()) {
                        auto &c = controls[player].value(); c.inputX = 0; c.inputY = 0; c.shoot = false; bool startPressed = false;
                        for (uint16_t ki = 0; ki < inp.keyCount; ++ki) {
                            using engine::R_Events::Key; Key code = static_cast<Key>(keys[ki]);
                            if (code == Key::Left || code == Key::Q) c.inputX = -1;
                            else if (code == Key::Right || code == Key::D) c.inputX = 1;
                            else if (code == Key::Space) c.shoot = true;
                            else if (code == Key::Enter) startPressed = true;
                        }
                        if (!gameStarted && startPressed) {
                            gameStarted = true; std::cout << "Server: game started by client ENTER" << std::endl; }
                    }
                }
            }
        }
    }

    void wrapPlayerHorizontally(registry &reg, entity_t player)
    {
        auto &poss = reg.get_components<component::position>(); auto &hbsAll = reg.get_components<component::hitbox>();
        if (player < poss.size() && poss[player] && poss[player].has_value() && player < hbsAll.size() && hbsAll[player] && hbsAll[player].has_value()) {
            auto &pp = poss[player].value(); const auto &phb = hbsAll[player].value();
            float left = pp.x + phb.offset_x; float right = left + phb.width;
            if (left > SCREEN_W) pp.x = -phb.width - phb.offset_x; else if (right < 0.f) pp.x = SCREEN_W - phb.offset_x;
        }
    }

    void updateMovingPlatforms(registry &reg)
    {
        auto &platsAll = reg.get_components<component::platform>(); auto &velsAll = reg.get_components<component::velocity>(); auto &hbs = reg.get_components<component::hitbox>(); auto &poss = reg.get_components<component::position>();
        for (size_t e = 0; e < platsAll.size(); ++e) {
            if (!(e < platsAll.size() && platsAll[e] && platsAll[e].has_value())) continue; if (platsAll[e].value().kind != 1) continue;
            if (!(e < poss.size() && poss[e] && poss[e].has_value())) continue; if (!(e < velsAll.size() && velsAll[e] && velsAll[e].has_value())) continue; if (!(e < hbs.size() && hbs[e] && hbs[e].has_value())) continue;
            auto &p = poss[e].value(); auto &v = velsAll[e].value(); const auto &hb = hbs[e].value();
            float left = p.x + hb.offset_x; float right = left + hb.width;
            if (left < 0.f) { p.x = -hb.offset_x; v.vx = std::abs(v.vx); }
            else if (right > SCREEN_W) { p.x = SCREEN_W - hb.width - hb.offset_x; v.vx = -std::abs(v.vx); }
        }
    }

    void spawnAhead(registry &reg, std::vector<entity_t> &platforms, std::vector<entity_t> &enemies, std::mt19937 &rng,
                    std::uniform_real_distribution<float> &xDist, std::uniform_real_distribution<float> &spacing,
                    std::uniform_int_distribution<int> &enemySpawnRoll, std::uniform_real_distribution<float> &enemyOffset01,
                    std::uniform_int_distribution<int> &kindDist, std::uniform_int_distribution<int> &trampolineRoll,
                    float &minPlatformY, entity_t player)
    {
    auto &poss = reg.get_components<component::position>(); if (!(player < poss.size() && poss[player] && poss[player].has_value())) return; float playerY = poss[player].value().y; const float SPAWN_AHEAD = 300.0f;
        while (playerY < minPlatformY + SPAWN_AHEAD) {
            float nx = xDist(rng); float gap = spacing(rng); minPlatformY -= gap; auto pe = reg.spawn_entity();
            reg.add_component(pe, component::position{nx, minPlatformY}); reg.add_component(pe, component::hitbox{PLATFORM_W_C, PLATFORM_H_C});
            int kd = kindDist(rng); uint8_t pkind = 0; if (kd <= 5) pkind = 0; else if (kd <= 7) pkind = 1; else if (kd == 8) pkind = 2; else pkind = 0;
            if (pkind == 0 && trampolineRoll(rng) == 0) pkind = 3;
            reg.add_component(pe, component::platform{pkind}); reg.add_component(pe, component::entity_kind::decor);
            if (pkind == 1) { float mv = (rng() % 2 == 0) ? 40.0f : -40.0f; reg.add_component(pe, component::velocity{mv, 0.0f}); }
            if (pkind == 2) { reg.add_component(pe, component::health{1}); }
            platforms.push_back(pe);
            if ((pkind == 0 || pkind == 1) && enemySpawnRoll(rng) == 0) {
                auto &posAll = reg.get_components<component::position>(); auto &hbAll = reg.get_components<component::hitbox>();
                if (posAll[pe] && posAll[pe].has_value() && hbAll[pe] && hbAll[pe].has_value()) {
                    const auto &pp = posAll[pe].value(); float ex = enemyOffset01(rng) * (SCREEN_W - ENEMY_W_C); float ey = pp.y - ENEMY_H_C - (10.0f + enemyOffset01(rng) * 40.0f);
                    entity_t en = reg.spawn_entity(); reg.add_component(en, component::position{ex, ey}); float drift = 20.0f + enemyOffset01(rng) * 20.0f; float dir = (rng() % 2 == 0) ? 1.0f : -1.0f;
                    reg.add_component(en, component::velocity{dir * drift, 0.0f}); reg.add_component(en, component::hitbox{ ENEMY_W_C, ENEMY_H_C }); reg.add_component(en, component::entity_kind::enemy);
                    enemies.push_back(en);
                }
            }
        }
    }

    void prunePlatforms(registry &reg, std::vector<entity_t> &platforms, entity_t player)
    {
        const float PRUNE_BELOW = SCREEN_H * 0.6f; auto &posArr = reg.get_components<component::position>(); auto &playerPosArr = reg.get_components<component::position>();
        if (!(player < playerPosArr.size() && playerPosArr[player] && playerPosArr[player].has_value())) return; float playerY = playerPosArr[player].value().y;
        std::vector<entity_t> kept; kept.reserve(platforms.size());
        for (auto e : platforms) { if (e >= posArr.size() || !posArr[e] || !posArr[e].has_value()) continue; float py = posArr[e].value().y; if (py > playerY + PRUNE_BELOW) { reg.kill_entity(e);} else { kept.push_back(e);} }
        platforms.swap(kept);
    }

    void resetPlatforms(registry &reg, std::vector<entity_t> &platforms,
                        float startW, float startH, float startX, float startY,
                        float &minPlatformY, float &lastSafeY)
    {
        for (auto e : platforms) { reg.kill_entity(e); }
        platforms.clear();
        entity_t startPlat = reg.spawn_entity();
        reg.add_component(startPlat, component::position{startX, startY});
        reg.add_component(startPlat, component::hitbox{startW, startH});
        reg.add_component(startPlat, component::platform{0});
        reg.add_component(startPlat, component::entity_kind::decor);
        platforms.push_back(startPlat);
        minPlatformY = startY; lastSafeY = startY;
    }

    void limitPlatformsCount(registry &reg, std::vector<entity_t> &platforms)
    {
        const size_t MAX_PLATFORMS = 800; if (platforms.size() <= MAX_PLATFORMS) return; size_t removeCount = platforms.size() - MAX_PLATFORMS; for (size_t i = 0; i < removeCount; ++i) reg.kill_entity(platforms[i]); platforms.erase(platforms.begin(), platforms.begin() + removeCount);
    }

    void resolvePlayerPlatformCollisions(registry &reg, entity_t player, float fixedDt, float &outLastSafeY)
    {
        auto &poss = reg.get_components<component::position>(); auto &vels = reg.get_components<component::velocity>(); auto &hbs = reg.get_components<component::hitbox>(); auto &plats = reg.get_components<component::platform>();
        if (!(player < poss.size() && poss[player] && player < vels.size() && vels[player] && player < hbs.size() && hbs[player])) return; float prevY = poss[player].value().y - vels[player].value().vy * fixedDt; float prevBottom = prevY + hbs[player].value().height; float curBottom = poss[player].value().y + hbs[player].value().height; float playerLeft = poss[player].value().x + hbs[player].value().offset_x; float playerRight = playerLeft + hbs[player].value().width;
        for (size_t e = 0; e < poss.size(); ++e) {
            if (!(e < poss.size() && poss[e] && poss[e].has_value())) continue; if (!(e < hbs.size() && hbs[e] && hbs[e].has_value())) continue; if (!(e < plats.size() && plats[e] && plats[e].has_value())) continue; float platTop = poss[e].value().y; if (!(platTop >= prevBottom - 50.0f && platTop <= curBottom + 200.0f)) continue;
            float platLeft = poss[e].value().x + hbs[e].value().offset_x; float platRight = platLeft + hbs[e].value().width; bool overlapX = (playerRight > platLeft + 1.0f) && (playerLeft < platRight - 1.0f);
            if (vels[player].value().vy > 0 && overlapX && prevBottom <= platTop + 5.0f && curBottom > platTop) {
                poss[player].value().y = platTop - hbs[player].value().height; uint8_t pkind = plats[e].value().kind; outLastSafeY = platTop;
                if (pkind == 3) {
                    vels[player].value().vy = PLAYER_JUMP_V * 1.5f;
                } else if (pkind == 2) {
                    vels[player].value().vy = PLAYER_JUMP_V;
                    reg.kill_entity(reg.entity_from_index(e));
                } else if (pkind == 1) {
                    vels[player].value().vy = PLAYER_JUMP_V;
                    auto &velsAll = reg.get_components<component::velocity>();
                    if (e < velsAll.size() && velsAll[e] && velsAll[e].has_value()) {
                        vels[player].value().vx += velsAll[e].value().vx * 0.5f;
                    }
                } else {
                    vels[player].value().vy = PLAYER_JUMP_V;
                }
                break;
            }
        }
    }

    void resolvePlayerEnemyCollisions(registry &reg, std::vector<entity_t> &enemies, entity_t player, float playerSpawnX, float playerSpawnY, bool &gameStarted, float fixedDt)
    {
        auto &poss = reg.get_components<component::position>(); auto &vels = reg.get_components<component::velocity>(); auto &hbs = reg.get_components<component::hitbox>();
        if (!(player < poss.size() && poss[player] && player < vels.size() && vels[player] && player < hbs.size() && hbs[player])) return; float prevY = poss[player].value().y - vels[player].value().vy * fixedDt; float prevBottom = prevY + hbs[player].value().height; float curBottom = poss[player].value().y + hbs[player].value().height; float playerLeft = poss[player].value().x + hbs[player].value().offset_x; float playerRight = playerLeft + hbs[player].value().width; float prevTop = prevY; float curTop = poss[player].value().y;
        std::vector<entity_t> keptEnemies; keptEnemies.reserve(enemies.size());
        for (auto en : enemies) {
            if (!(en < poss.size() && poss[en] && poss[en].has_value())) { continue; } if (!(en < hbs.size() && hbs[en] && hbs[en].has_value())) { continue; } float eTop = poss[en].value().y; float eBottom = eTop + hbs[en].value().height; float eLeft = poss[en].value().x + hbs[en].value().offset_x; float eRight = eLeft + hbs[en].value().width; bool overlapX = (playerRight > eLeft + 1.0f) && (playerLeft < eRight - 1.0f);
            bool stomp = (vels[player].value().vy > 0) && overlapX && (prevBottom <= eTop + 5.0f) && (curBottom > eTop);
            bool head = (vels[player].value().vy < 0) && overlapX && (prevTop >= eBottom - 5.0f) && (curTop < eBottom);
            if (stomp) { reg.kill_entity(en); vels[player].value().vy = PLAYER_JUMP_V * STOMP_MULT; poss[player].value().y = eTop - hbs[player].value().height; }
            else if (head) {
                std::cout << "Server: player died (head bump) -> respawn and wait" << std::endl; poss[player].value().x = playerSpawnX; poss[player].value().y = playerSpawnY; vels[player].value().vx = 0.0f; vels[player].value().vy = 0.0f; gameStarted = false; for (auto k : enemies) reg.kill_entity(k); enemies.clear();
                auto &projArrClr = reg.get_components<component::projectile_tag>(); for (size_t i = 0; i < projArrClr.size(); ++i) { if (projArrClr[i] && projArrClr[i].has_value()) reg.kill_entity(reg.entity_from_index(i)); }
                keptEnemies.clear(); break;
            } else { keptEnemies.push_back(en); }
        }
        enemies.swap(keptEnemies);
    }

    void clampEnemiesToBounds(registry &reg, std::vector<entity_t> &enemies)
    {
    auto &posAll = reg.get_components<component::position>(); auto &hbAll = reg.get_components<component::hitbox>(); auto &velAll = reg.get_components<component::velocity>();
        std::vector<entity_t> kept; kept.reserve(enemies.size());
    for (auto en : enemies) { if (!(en < posAll.size() && posAll[en] && posAll[en].has_value())) continue; if (!(en < hbAll.size() && hbAll[en] && hbAll[en].has_value())) continue; if (!(en < velAll.size() && velAll[en] && velAll[en].has_value())) continue; auto &ep = posAll[en].value(); auto &ev = velAll[en].value(); const auto &ehb = hbAll[en].value(); float leftEdge = 0.0f; float rightEdge = SCREEN_W - ehb.width; if (ep.x < leftEdge) { ep.x = leftEdge; ev.vx = std::abs(ev.vx);} else if (ep.x > rightEdge) { ep.x = rightEdge; ev.vx = -std::abs(ev.vx);} kept.push_back(en);} enemies.swap(kept);
    }

    void resolveProjectileEnemyHits(registry &reg, std::vector<entity_t> &enemies)
    {
        auto &posAll = reg.get_components<component::position>(); auto &hbAll = reg.get_components<component::hitbox>(); auto &projAll = reg.get_components<component::projectile_tag>();
        std::vector<entity_t> deadEnemies; std::vector<entity_t> deadProjectiles;
        for (auto en : enemies) {
            if (!(en < posAll.size() && posAll[en] && posAll[en].has_value())) continue; if (!(en < hbAll.size() && hbAll[en] && hbAll[en].has_value())) continue; float eLeft = posAll[en].value().x + hbAll[en].value().offset_x; float eTop = posAll[en].value().y + hbAll[en].value().offset_y; float eRight = eLeft + hbAll[en].value().width; float eBottom = eTop + hbAll[en].value().height;
            for (size_t i = 0; i < projAll.size(); ++i) { if (!projAll[i] || !projAll[i].has_value()) continue; entity_t projE = reg.entity_from_index(i); if (!(posAll[projE] && hbAll[projE])) continue; float pLeft = posAll[projE].value().x + hbAll[projE].value().offset_x; float pTop = posAll[projE].value().y + hbAll[projE].value().offset_y; float pRight = pLeft + hbAll[projE].value().width; float pBottom = pTop + hbAll[projE].value().height; bool overlap = !(pRight <= eLeft || pLeft >= eRight || pBottom <= eTop || pTop >= eBottom); if (overlap) { deadEnemies.push_back(en); deadProjectiles.push_back(projE); break; } }
        }
        if (!deadEnemies.empty()) { std::vector<entity_t> kept; kept.reserve(enemies.size()); for (auto de : deadEnemies) reg.kill_entity(de); for (auto en : enemies) { bool isDead = false; for (auto de : deadEnemies) { if (de == en) { isDead = true; break; } } if (!isDead) kept.push_back(en);} enemies.swap(kept); for (auto dp : deadProjectiles) reg.kill_entity(dp); }
    }

    void cullProjectiles(registry &reg, entity_t player)
    {
        auto &projArr = reg.get_components<component::projectile_tag>(); auto &posArr = reg.get_components<component::position>(); std::vector<entity_t> toKillProj;
        for (size_t i = 0; i < projArr.size(); ++i) {
            if (!projArr[i] || !projArr[i].has_value()) continue; auto &pt = projArr[i].value(); if (pt.lifetime > 0) --pt.lifetime; if (pt.lifetime == 0) { toKillProj.push_back(reg.entity_from_index(i)); continue; }
            if (i < posArr.size() && posArr[i] && posArr[i].has_value()) {
                if (player < posArr.size() && posArr[player] && posArr[player].has_value()) { float pY = posArr[player].value().y; const float TOP_MARGIN = 1500.0f; const float BOTTOM_MARGIN = 2500.0f; float y = posArr[i].value().y; if (y < pY - TOP_MARGIN || y > pY + BOTTOM_MARGIN) { toKillProj.push_back(reg.entity_from_index(i)); } }
                else { if (posArr[i].value().y < -10000.0f || posArr[i].value().y > 10000.0f) { toKillProj.push_back(reg.entity_from_index(i)); } }
            }
        }
        for (auto e : toKillProj) reg.kill_entity(e);
    }

    std::vector<uint8_t> buildSnapshotPayload(registry &reg, uint32_t tick)
    {
        std::vector<EntityState> states; auto &positions = reg.get_components<component::position>(); auto &velocities = reg.get_components<component::velocity>(); auto &kinds = reg.get_components<component::entity_kind>(); auto &hitboxes = reg.get_components<component::hitbox>();
        for (size_t i = 0; i < positions.size(); ++i) {
            if (!positions[i] || !positions[i].has_value()) continue; EntityState st{}; st.entityId = static_cast<uint32_t>(i); st.x = positions[i].value().x; st.y = positions[i].value().y;
            if (i < velocities.size() && velocities[i] && velocities[i].has_value()) { st.vx = velocities[i].value().vx; st.vy = velocities[i].value().vy; } else { st.vx = st.vy = 0.f; }
            st.type = (i < kinds.size() && kinds[i] && kinds[i].has_value()) ? static_cast<uint8_t>(kinds[i].value()) : static_cast<uint8_t>(component::entity_kind::unknown);
            st.hp = 0; st.collided = false; auto &plats = reg.get_components<component::platform>(); st.platformType = (i < plats.size() && plats[i] && plats[i].has_value()) ? plats[i].value().kind : 0;
            if (i < hitboxes.size() && hitboxes[i] && hitboxes[i].has_value()) { st.hb_w = hitboxes[i].value().width; st.hb_h = hitboxes[i].value().height; st.hb_ox = hitboxes[i].value().offset_x; st.hb_oy = hitboxes[i].value().offset_y; }
            states.push_back(st);
        }
        Snapshot snap{tick, static_cast<uint16_t>(states.size())}; std::vector<uint8_t> payload(sizeof(Snapshot) + states.size() * sizeof(EntityState)); std::memcpy(payload.data(), &snap, sizeof(Snapshot)); std::memcpy(payload.data() + sizeof(Snapshot), states.data(), states.size() * sizeof(EntityState)); return payload;
    }
}

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

        std::vector<entity_t> platforms;
        std::vector<entity_t> enemies;
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
        reg.add_component(player, component::hitbox{60.0f, 90.0f});
        float playerSpawnX = START_PLATFORM_X + (START_PLATFORM_W * 0.5f) - 14.0f;
        float playerSpawnY = START_PLATFORM_Y - 90.0f;
        reg.add_component(player, component::position{playerSpawnX, playerSpawnY});
        reg.add_component(player, component::velocity{0.0f, 0.0f});
        reg.add_component(player, component::controllable{});
        reg.add_component(player, component::gravity{GRAVITY_C});
        reg.add_component(player, component::entity_kind::player);

        std::cout << "Server: spawned player entity id=" << static_cast<std::size_t>(player) << std::endl;
        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> xDist(0.0f, SCREEN_W - PLATFORM_W);
        std::uniform_real_distribution<float> spacing(80.0f, 120.0f);
        std::uniform_int_distribution<int> enemySpawnRoll(0, 19);
        std::uniform_real_distribution<float> enemyOffset01(0.0f, 1.0f);

        std::uniform_int_distribution<int> kindDist(0, 9);
        std::uniform_int_distribution<int> trampolineRoll(0, 9);

        float minPlatformY = START_PLATFORM_Y;

        using clock = std::chrono::steady_clock;
        auto lastTime = clock::now();
        auto lastClientActivity = clock::now();
        const auto clientTimeout = std::chrono::seconds(5);
        float accumulator = 0.f;
        const float fixedDt = 1.0f / 60.0f;
        uint32_t tick = 0;
        uint32_t lastShootTick = 0;
        const uint32_t SHOOT_COOLDOWN_TICKS = 8;
        const float ENEMY_W = 163.0f;
        const float ENEMY_H = 102.0f;
        const float ENEMY_SPEED = 35.0f;
        const float STOMP_MULT = 1.3f;

        float lastSafeY = playerSpawnY;
        bool fallGraceActive = false;
        auto fallGraceStart = clock::now();
        const float FALL_DEATH_DISTANCE = 600.0f;
        const int FALL_GRACE_MS = 700;

    std::cout << "Doodle server listening on port " << port << std::endl;

        waitForHandshake(io, sock, connected, lastSender, static_cast<uint16_t>(player));
        lastClientActivity = clock::now();

        std::cout << "Server: entering run loop" << std::endl;

        bool running = true;
        while (running) {
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

            io.poll();
            processIncomingPackets(sock, reg, player, gameStarted, lastSender, lastClientActivity);

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
                    v.vx = c.inputX * PLAYER_MOVE_S;

                    if (c.shoot && (tick - lastShootTick >= SHOOT_COOLDOWN_TICKS)) {
                        lastShootTick = tick;
                        auto &possArr = reg.get_components<component::position>();
                        auto &hbsArr = reg.get_components<component::hitbox>();
                        if (possArr[player] && possArr[player].has_value() && hbsArr[player] && hbsArr[player].has_value()) {
                            const auto &pp = possArr[player].value();
                            const auto &phb = hbsArr[player].value();

                            const float projW = 6.0f;
                            const float projH = 12.0f;
                            float spawnX = pp.x + phb.offset_x + (phb.width * 0.5f) - (projW * 0.5f);
                            float spawnY = pp.y - projH - 2.0f;

                            entity_t proj = reg.spawn_entity();
                            reg.add_component(proj, component::position{spawnX, spawnY});
                            reg.add_component(proj, component::velocity{0.f, -PROJECTILE_SPEED});
                            reg.add_component(proj, component::hitbox{projW, projH});
                            reg.add_component(proj, component::projectile_tag{static_cast<std::uint32_t>(player), 120u, 0.f, -1.f, PROJECTILE_SPEED, 1});
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

                cullProjectiles(reg, player);

                auto &poss = reg.get_components<component::position>();
                if (gameStarted) {
                    position_system(reg, poss, vels, fixedDt);

                    wrapPlayerHorizontally(reg, player);
                    updateMovingPlatforms(reg);
                }

                if (gameStarted && poss[player] && poss[player].has_value()) {
                    spawnAhead(reg, platforms, enemies, rng, xDist, spacing, enemySpawnRoll, enemyOffset01, kindDist, trampolineRoll, minPlatformY, player);
                    prunePlatforms(reg, platforms, player);
                    limitPlatformsCount(reg, platforms);
                }

                auto &hbArrPlayerCheck = reg.get_components<component::hitbox>();
                if (gameStarted && player < poss.size() && poss[player] && player < vels.size() && vels[player] && player < hbArrPlayerCheck.size() && hbArrPlayerCheck[player]) {
                    resolvePlayerPlatformCollisions(reg, player, fixedDt, lastSafeY);
                    bool prevGameStarted = gameStarted;
                    resolvePlayerEnemyCollisions(reg, enemies, player, playerSpawnX, playerSpawnY, gameStarted, fixedDt);
                    if (prevGameStarted && !gameStarted) {
                        resetPlatforms(reg, platforms, START_PLATFORM_W, START_PLATFORM_H, START_PLATFORM_X, START_PLATFORM_Y, minPlatformY, lastSafeY);
                    }
                }

                if (gameStarted) { clampEnemiesToBounds(reg, enemies); }

                resolveProjectileEnemyHits(reg, enemies);

                if (gameStarted && poss[player] && poss[player].has_value()) {
                    float py = poss[player].value().y;
                    if (py > lastSafeY + FALL_DEATH_DISTANCE) {
                        if (!fallGraceActive) { fallGraceActive = true; fallGraceStart = clock::now(); }
                        else {
                            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - fallGraceStart).count();
                            if (ms > FALL_GRACE_MS) {
                                std::cout << "Server: player fell -> respawn and wait" << std::endl;
                                poss[player].value().x = playerSpawnX; poss[player].value().y = playerSpawnY;
                                vels[player].value().vx = 0.0f; vels[player].value().vy = 0.0f;
                                gameStarted = false;
                                for (auto k : enemies) reg.kill_entity(k);
                                enemies.clear();
                                auto &projArrClr = reg.get_components<component::projectile_tag>();
                                for (size_t i = 0; i < projArrClr.size(); ++i) { if (projArrClr[i] && projArrClr[i].has_value()) reg.kill_entity(reg.entity_from_index(i)); }
                                resetPlatforms(reg, platforms, START_PLATFORM_W, START_PLATFORM_H, START_PLATFORM_X, START_PLATFORM_Y, minPlatformY, lastSafeY);
                                fallGraceActive = false;
                            }
                        }
                    } else {
                        fallGraceActive = false;
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

            auto payload = buildSnapshotPayload(reg, tick);
            PacketHeader hdr{SNAPSHOT, static_cast<uint16_t>(payload.size()), tick};

            if (!lastSender.address.empty() && lastSender.port != 0) {
                try {
                    sock.send(hdr, payload, lastSender);
                } catch (const std::exception &ex) {
                    (void)ex;
                }
            }

        }
    return 0;
}

