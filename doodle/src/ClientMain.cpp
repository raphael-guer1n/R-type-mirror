#include "engine/Engine.hpp"
#include "common/Packets.hpp"
#include <iostream>
#include <unordered_set>
#include <vector>
#include <cstring>

#include "engine/network/IoContext.hpp"
#include "engine/network/UdpSocket.hpp"
#include "engine/network/Endpoint.hpp"
#include "engine/renderer/Texture.hpp"
#include "engine/audio/AudioManager.hpp"
#include "common/Layers.hpp"

using namespace engine;
using namespace engine::R_Graphic;

namespace {
    constexpr bool VERBOSE = true;
    constexpr int SCREEN_W = 480;
    constexpr int SCREEN_H = 800;
    constexpr uint32_t INPUT_SEND_HZ = 20;
    constexpr uint32_t INPUT_SEND_MS = 1000u / INPUT_SEND_HZ;
    constexpr bool SHOW_HITBOXES = false;

    struct Textures {
        std::shared_ptr<engine::R_Graphic::Texture> player;
        std::shared_ptr<engine::R_Graphic::Texture> tileRegular;
        std::shared_ptr<engine::R_Graphic::Texture> tileMoving;
        std::shared_ptr<engine::R_Graphic::Texture> tileGhost;
        std::shared_ptr<engine::R_Graphic::Texture> playerProj;
        std::shared_ptr<engine::R_Graphic::Texture> decor;
        std::shared_ptr<engine::R_Graphic::Texture> monster;
        std::shared_ptr<engine::R_Graphic::Texture> trampoline;
    };

    void handleInputEvents(engine::R_Graphic::Window &window,
                           bool &running,
                           std::unordered_set<int32_t> &pressed)
    {
        auto events = window.pollEvents(running);
        for (auto &ev : events) {
            if (ev.type == engine::R_Events::Type::Quit) {
                running = false; break;
            }
            if (ev.type == engine::R_Events::Type::KeyDown) {
                pressed.insert(static_cast<int32_t>(ev.key.code));
                if (ev.key.code == engine::R_Events::Key::Space) {
                    engine::audio::AudioManager::instance().playSound("shoot");
                }
            }
            if (ev.type == engine::R_Events::Type::KeyUp) {
                pressed.erase(static_cast<int32_t>(ev.key.code));
            }
        }
    }

    void maybeSendConnectReq(bool connected,
                             uint32_t nowMs,
                             uint32_t &lastConnReqMs,
                             engine::net::UdpSocket &sock,
                             const engine::net::Endpoint &serverEndpoint)
    {
        if (connected)
            return;
        if (nowMs - lastConnReqMs < 1000u)
            return;
        lastConnReqMs = nowMs;
        ConnectReq req{0u};
        std::vector<uint8_t> buf(sizeof(ConnectReq));
        std::memcpy(buf.data(), &req, sizeof(ConnectReq));
        PacketHeader h{CONNECT_REQ, static_cast<uint16_t>(buf.size()), 0};
        try { sock.send(h, buf, serverEndpoint);
            if (VERBOSE) std::cout << "Client: sent CONNECT_REQ" << std::endl;
        } catch (...) {}
    }

    void maybeSendInput(bool connected,
                        uint32_t nowMs,
                        uint32_t &lastInputSendMs,
                        uint32_t &tick,
                        const std::unordered_set<int32_t> &pressed,
                        engine::net::UdpSocket &sock,
                        const engine::net::Endpoint &serverEndpoint)
    {
        if (!connected)
            return;
        if (nowMs - lastInputSendMs < INPUT_SEND_MS)
            return;
        lastInputSendMs = nowMs;
        InputPacket inp{};
        inp.clientId = 0;
        inp.tick = tick++;
        inp.keyCount = static_cast<uint16_t>(pressed.size());
        std::vector<int32_t> keys; keys.reserve(pressed.size());
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

    void parseSnapshotPayload(registry &reg,
                              const std::vector<uint8_t> &payload,
                              uint32_t &lastParseLogMs)
    {
        Snapshot snap{};
        std::memcpy(&snap, payload.data(), sizeof(Snapshot));
        if (VERBOSE)
            std::cout << "Client: received SNAPSHOT tick=" << snap.tick << " entityCount=" << snap.entityCount
                      << " payload=" << payload.size() << std::endl;

        size_t n = snap.entityCount;
        size_t payloadEntitiesBytes = payload.size() - sizeof(Snapshot);
        size_t countWithPlatform = (payloadEntitiesBytes) / sizeof(EntityState);

        int playerCount = 0;
        auto &positions = reg.get_components<component::position>();
        auto &velocities = reg.get_components<component::velocity>();
        auto &hitboxes = reg.get_components<component::hitbox>();
        auto &platforms = reg.get_components<component::platform>();
        auto &kinds = reg.get_components<component::entity_kind>();

        auto applyStates = [&](size_t usable) {
            std::vector<uint8_t> present;
            const EntityState *states = reinterpret_cast<const EntityState *>(payload.data() + sizeof(Snapshot));
            for (size_t i = 0; i < usable; ++i) {
                const EntityState &es = states[i];
                size_t idx = es.entityId;
                if (idx >= present.size())
                    present.resize(idx + 1, 0);
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
                hitboxes[idx] = component::hitbox{ es.hb_w, es.hb_h, es.hb_ox, es.hb_oy };
                kinds[idx] = static_cast<component::entity_kind>(es.type);
                platforms[idx] = component::platform{es.platformType};
                if (kinds[idx] == component::entity_kind::player)
                    ++playerCount;
            }
            size_t maxSize = positions.size();
            for (size_t i = 0; i < maxSize; ++i) {
                bool isPresent = (i < present.size()) ? (present[i] != 0) : false;
                if (!isPresent) {
                    if (i < positions.size())
                        positions.erase(i);
                    if (i < velocities.size())
                        velocities.erase(i);
                    if (i < hitboxes.size())
                        hitboxes.erase(i);
                    if (i < platforms.size())
                        platforms.erase(i);
                    if (i < kinds.size())
                        kinds.erase(i);
                }
            }
        };

        if (countWithPlatform >= n) {
            applyStates(n);
        } else {
            size_t usable = std::min(n, countWithPlatform);
            if (usable > 0)
                applyStates(usable);
            if (VERBOSE)
                std::cerr << "Client: snapshot size mismatch (hdr.entityCount=" << n
                          << ", bytes=" << payloadEntitiesBytes << ") parsed=" << usable << " entries" << std::endl;
        }

        uint32_t nowLog = SDL_GetTicks();
        if (VERBOSE || nowLog - lastParseLogMs > 1000u) {
            lastParseLogMs = nowLog;
            std::cout << "Client: parsed snapshot tick=" << snap.tick << " entities=" << n
                      << " positionsSize=" << positions.size() << " kindsSize=" << kinds.size() << std::endl;
        }
    }

    void processIncoming(registry &reg,
                         engine::net::UdpSocket &sock,
                         bool &connected,
                         uint32_t &lastParseLogMs)
    {
        engine::net::Endpoint senderEndpoint; PacketHeader hdr; std::vector<uint8_t> payload;
        while (sock.PollPacket(hdr, payload, senderEndpoint)) {
            if (VERBOSE)
                std::cout << "Client: received packet type=" << int(hdr.type) << " size=" << hdr.size << std::endl;
            if (hdr.type == CONNECT_ACK && payload.size() >= sizeof(ConnectAck)) {
                connected = true;
                if (VERBOSE) {
                    ConnectAck ack{}; std::memcpy(&ack, payload.data(), sizeof(ConnectAck));
                    std::cout << "Client: CONNECT_ACK tickRate=" << ack.tickRate << " playerId=" << ack.playerEntityId << std::endl;
                }
            } else if (hdr.type == SNAPSHOT && payload.size() >= sizeof(Snapshot)) {
                parseSnapshotPayload(reg, payload, lastParseLogMs);
            }
        }
    }

    void updateMonstersMusic(const registry &reg, bool &loopPlaying)
    {
        auto &kinds = const_cast<registry&>(reg).get_components<component::entity_kind>();
        size_t enemyCount = 0;
        for (size_t i = 0; i < kinds.size(); ++i) {
            if (kinds[i] && kinds[i].has_value() && kinds[i].value() == component::entity_kind::enemy) ++enemyCount;
        }
        if (enemyCount > 0 && !loopPlaying) {
            engine::audio::AudioManager::instance().playMusic("monsters", true);
            loopPlaying = true;
        } else if (enemyCount == 0 && loopPlaying) {
            engine::audio::AudioManager::instance().stopMusic();
            loopPlaying = false;
        }
    }

    void updateKinematicsSounds(const registry &reg,
                                ssize_t playerIdx,
                                float &prevVy,
                                float &prevY,
                                bool &havePrevKinematics,
                                uint32_t &lastFallSoundMs)
    {
        auto &velocities = const_cast<registry&>(reg).get_components<component::velocity>();
        auto &positions = const_cast<registry&>(reg).get_components<component::position>();
        if (playerIdx < 0 || static_cast<size_t>(playerIdx) >= velocities.size() || !velocities[playerIdx] || !positions[playerIdx])
            return;
        float vy = velocities[playerIdx].value().vy;
        float y = positions[playerIdx].value().y;
        if (havePrevKinematics) {
            if (prevVy > -100.0f && vy <= -800.0f) {
                if (vy <= -1100.0f)
                    engine::audio::AudioManager::instance().playSound("trampoline");
                else engine::audio::AudioManager::instance().playSound("jump");
            }
            if (prevVy < 0.0f && std::abs(vy) < 1.0f && (y - prevY) > 60.0f) {
                engine::audio::AudioManager::instance().playSound("monster-crash");
            }
            uint32_t nowMs = SDL_GetTicks();
            if (vy > 600.0f && (y - prevY) > 40.0f && (nowMs - lastFallSoundMs) > 1500u) {
                engine::audio::AudioManager::instance().playSound("fall");
                lastFallSoundMs = nowMs;
            }
        }
        prevVy = vy; prevY = y; havePrevKinematics = true;
    }

    float computeCameraY(const registry &reg, float cameraPrev)
    {
        auto &kinds = const_cast<registry&>(reg).get_components<component::entity_kind>();
        auto &positions = const_cast<registry&>(reg).get_components<component::position>();
        ssize_t playerIdx = -1;
        for (size_t i = 0; i < kinds.size(); ++i) {
            if (kinds[i] && kinds[i].has_value() && kinds[i].value() == component::entity_kind::player) { playerIdx = static_cast<ssize_t>(i); break; }
        }
        float cameraY = cameraPrev;
        if (playerIdx >= 0 && playerIdx < static_cast<ssize_t>(positions.size()) && positions[playerIdx]) {
            float targetY = positions[playerIdx].value().y - (SCREEN_H / 2.0f);
            const float SMOOTH = 0.12f;
            cameraY = cameraPrev + (targetY - cameraPrev) * SMOOTH;
        }
        return cameraY;
    }

    void drawScene(Window &window,
                   Renderer &renderer,
                   registry &reg,
                   const Textures &tex,
                   float cameraY,
                   uint32_t &lastParseLogMs)
    {
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

        auto drawPlatformWithOverlay = [&](size_t i) {
            auto p = positions[i].value();
            auto hb = hitboxes[i].value();
            std::shared_ptr<engine::R_Graphic::Texture> baseTex = tex.tileRegular;
            bool overlayTrampoline = false;
            if (i < platforms.size() && platforms[i] && platforms[i].has_value()) {
                uint8_t pk = platforms[i].value().kind;
                switch (pk) {
                    case 1: baseTex = tex.tileMoving ? tex.tileMoving : tex.tileRegular; break;
                    case 2: baseTex = tex.tileGhost ? tex.tileGhost : tex.tileRegular; break;
                    case 3: baseTex = tex.tileRegular ? tex.tileRegular : tex.decor; overlayTrampoline = true; break;
                    default: baseTex = tex.tileRegular ? tex.tileRegular : tex.decor; break;
                }
            }
            if (baseTex) {
                auto size = baseTex->getSize();
                double texW = static_cast<double>(size.x);
                double texH = static_cast<double>(size.y);
                double texPosX = p.x + hb.offset_x + (hb.width * 0.5) - (texW * 0.5);
                double texPosY = p.y + hb.offset_y + (hb.height * 0.5) - (texH * 0.5);
                baseTex->setPosition(texPosX, texPosY - cameraY);
                baseTex->draw(window, nullptr);
                if (overlayTrampoline && tex.trampoline) {
                    auto tsize = tex.trampoline->getSize();
                    double trampW = static_cast<double>(tsize.x);
                    double trampH = static_cast<double>(tsize.y);
                    double platformTexTop = texPosY;
                    double drawX = p.x + hb.offset_x + (hb.width * 0.5) - (trampW * 0.5);
                    const double TRAMPOLINE_GAP = 0.0;
                    double drawY = platformTexTop - trampH - TRAMPOLINE_GAP;
                    tex.trampoline->setPosition(drawX, drawY - cameraY);
                    tex.trampoline->draw(window, nullptr);
                }
            }
        };

        for (size_t i = 0; i < positions.size(); ++i) {
            if (!(positions[i] && hitboxes[i] && kinds[i] && kinds[i].has_value()))
                continue;
            if (kinds[i].value() != component::entity_kind::decor)
                continue;
            drawPlatformWithOverlay(i);
        }
        for (size_t i = 0; i < positions.size(); ++i) {
            if (!(positions[i] && hitboxes[i] && kinds[i] && kinds[i].has_value()))
                continue;
            if (kinds[i].value() != component::entity_kind::enemy)
                continue;
            auto p = positions[i].value(); auto hb = hitboxes[i].value(); auto useTex = tex.monster;
            if (useTex) {
                auto size = useTex->getSize();
                double texW = static_cast<double>(size.x); (void)texW;
                double texH = static_cast<double>(size.y); (void)texH;
                double texPosX = p.x + hb.offset_x + (hb.width * 0.5) - (size.x * 0.5);
                double texPosY = p.y + hb.offset_y + (hb.height * 0.5) - (size.y * 0.5);
                useTex->setPosition(texPosX, texPosY - cameraY);
                useTex->draw(window, nullptr);
            } else {
                renderer.setDrawColor(200, 50, 50, 255);
                renderer.fillRect(static_cast<int>(p.x), static_cast<int>(p.y - cameraY), static_cast<int>(hb.width), static_cast<int>(hb.height));
            }
        }
        for (size_t i = 0; i < positions.size(); ++i) {
            if (!(positions[i] && hitboxes[i] && kinds[i] && kinds[i].has_value()))
                continue;
            if (kinds[i].value() != component::entity_kind::player)
                continue;
            auto p = positions[i].value(); auto hb = hitboxes[i].value(); auto useTex = tex.player;
            if (useTex) {
                auto size = useTex->getSize();
                double texPosX = p.x + hb.offset_x + (hb.width * 0.5) - (size.x * 0.5);
                double texPosY = p.y + hb.offset_y + (hb.height * 0.5) - (size.y * 0.5);
                useTex->setPosition(texPosX, texPosY - cameraY);
                useTex->draw(window, nullptr);
            } else {
                renderer.setDrawColor(0, 200, 0, 255);
                renderer.fillRect(static_cast<int>(p.x), static_cast<int>(p.y - cameraY), static_cast<int>(hb.width), static_cast<int>(hb.height));
            }
        }
        for (size_t i = 0; i < positions.size(); ++i) {
            if (!(positions[i] && hitboxes[i] && kinds[i] && kinds[i].has_value()))
                continue;
            if (kinds[i].value() != component::entity_kind::playerProjectile)
                continue;
            auto p = positions[i].value(); auto hb = hitboxes[i].value(); auto useTex = tex.playerProj;
            if (useTex) {
                auto size = useTex->getSize();
                double texPosX = p.x + hb.offset_x + (hb.width * 0.5) - (size.x * 0.5);
                double texPosY = p.y + hb.offset_y + (hb.height * 0.5) - (size.y * 0.5);
                useTex->setPosition(texPosX, texPosY - cameraY);
                useTex->draw(window, nullptr);
            } else {
                renderer.setDrawColor(255, 60, 60, 255);
                renderer.fillRect(static_cast<int>(p.x), static_cast<int>(p.y - cameraY), static_cast<int>(hb.width), static_cast<int>(hb.height));
            }
        }

        for (size_t i = 0; i < positions.size(); ++i) {
            if (!(positions[i] && hitboxes[i]))
                continue;
            bool hasKind = (i < kinds.size() && kinds[i] && kinds[i].has_value());
            if (hasKind) {
                auto k = kinds[i].value();
                if (k == component::entity_kind::decor || k == component::entity_kind::enemy || k == component::entity_kind::player || k == component::entity_kind::playerProjectile)
                    continue;
            }
            auto p = positions[i].value(); auto hb = hitboxes[i].value();
            renderer.setDrawColor(80, 80, 80, 255);
            renderer.fillRect(static_cast<int>(p.x), static_cast<int>(p.y - cameraY), static_cast<int>(hb.width), static_cast<int>(hb.height));
        }

        if (SHOW_HITBOXES && playerIdx >= 0 && positions[playerIdx] && hitboxes[playerIdx]) {
            auto p = positions[playerIdx].value(); auto hb = hitboxes[playerIdx].value();
            int screenX = static_cast<int>(p.x); int screenY = static_cast<int>(p.y - cameraY);
            renderer.setDrawColor(0, 255, 0, 255);
            renderer.drawRect(screenX - 2, screenY - 2, static_cast<int>(hb.width) + 4, static_cast<int>(hb.height) + 4);
        }

        renderer.display();
    }
}

int main(int argc, char **argv) {
    try {
        std::string serverIp = "127.0.0.1";
        unsigned short serverPort = 4242;
        if (argc > 1)
            serverIp = argv[1];
        if (argc > 2)
            serverPort = static_cast<unsigned short>(std::atoi(argv[2]));

    Window window("Doodle - Client", SCREEN_W, SCREEN_H);
        Renderer renderer(window);

    engine::audio::AudioManager::instance().loadConfig("./assets/audio/audio_config.json");

    Textures tex;
    tex.player = std::make_shared<engine::R_Graphic::Texture>(window, "./assets/assets (reworked)/doodle.png");
    tex.tileRegular = std::make_shared<engine::R_Graphic::Texture>(window, "./assets/assets (reworked)/tile_regular.png");
    tex.tileMoving = std::make_shared<engine::R_Graphic::Texture>(window, "./assets/assets (reworked)/tile_moving.png");
    tex.tileGhost = std::make_shared<engine::R_Graphic::Texture>(window, "./assets/assets (reworked)/tile_ghost.png");
    tex.trampoline = std::make_shared<engine::R_Graphic::Texture>(window, "./assets/assets (reworked)/trampoline.png");
    tex.monster = std::make_shared<engine::R_Graphic::Texture>(window, "./assets/assets (reworked)/monster.png");
    tex.playerProj = nullptr;
    tex.decor = nullptr;

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

        uint32_t lastInputSendMs = SDL_GetTicks();
        uint32_t lastParseLogMs = SDL_GetTicks();
        float cameraYPrev = 0.0f;

        bool monstersLoopPlaying = false;
        float prevVy = 0.0f;
        float prevY = 0.0f;
        bool havePrevKinematics = false;
        uint32_t lastFallSoundMs = 0;

    while (running) {
            io.poll();
            handleInputEvents(window, running, pressed);

            if (pressed != prevPressed) {
                std::cout << "Client: pressed keys:";
                for (auto k : pressed)
                    std::cout << ' ' << k;
                std::cout << "\n";
                prevPressed = pressed;
            }

            uint32_t nowMs = SDL_GetTicks();
            maybeSendConnectReq(connected, nowMs, lastConnReqMs, sock, serverEndpoint);
            maybeSendInput(connected, nowMs, lastInputSendMs, tick, pressed, sock, serverEndpoint);

            processIncoming(reg, sock, connected, lastParseLogMs);

            auto &kinds_ref = reg.get_components<component::entity_kind>();
            ssize_t playerIdx = -1;
            for (size_t i = 0; i < kinds_ref.size(); ++i) {
                if (kinds_ref[i] && kinds_ref[i].has_value() && kinds_ref[i].value() == component::entity_kind::player) {
                    playerIdx = static_cast<ssize_t>(i);
                    break;
                }
            }
            updateMonstersMusic(reg, monstersLoopPlaying);
            updateKinematicsSounds(reg, playerIdx, prevVy, prevY, havePrevKinematics, lastFallSoundMs);

            float cameraY = computeCameraY(reg, cameraYPrev);
            drawScene(window, renderer, reg, tex, cameraY, lastParseLogMs);
            cameraYPrev = cameraY;

            SDL_Delay(16);
        }

        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Client error: " << e.what() << std::endl;
        return 1;
    }
}
