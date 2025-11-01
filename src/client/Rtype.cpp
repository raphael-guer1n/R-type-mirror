#include <iostream>
#include <SDL.h>
#include "Rtype.hpp"
#include "Background.hpp"
#include "engine/ecs/Components.hpp"
#include "common/Components_client.hpp"
#include "common/Components_client_sdl.hpp"
#include "common/Packets.hpp"
#include "engine/network/UdpSocket.hpp"
#include "common/Accessibility.hpp"
#include "engine/ecs/Systems.hpp"
#include "Background.hpp"
#include "Hud.hpp"
#include "common/Systems_client_sdl.hpp"
#include "engine/audio/AudioManager.hpp"
#include "common/Layers.hpp"
#include "engine/renderer/Error.hpp"
#include "engine/profiling/Profiler.hpp"
#include "engine/profiling/ProfilerOverlay.hpp"

R_Type::Rtype::Rtype()
    : _app("R-Type", 1920, 1080)
{
    engine::audio::AudioManager::instance().loadConfig("./configs/audio_config.json");
    
    _profilerOverlay = std::make_unique<Engine::Profiling::ProfilerOverlay>();
    if (_profilerOverlay->initialize(_app.getWindow().getRenderer(), "Assets/fonts/arial.ttf")) {
        Engine::Profiling::ProfilerDisplayConfig config;
        config.showFPS = true;
        config.showFrameTime = true;
        config.showMemory = true;
        config.showCPU = false;
        config.showNetwork = true;
        config.showWorld = true;
        config.posX = 10;
        config.posY = 10;
        _profilerOverlay->setConfig(config);
        std::cout << "[Profiling] Overlay initialized\n";
    }
    if (TTF_WasInit() == 0) {
        if (TTF_Init() == -1) {
            std::cerr << "[UI] Erreur TTF_Init: " << TTF_GetError() << "\n";
        }
    }
    _uiFont = TTF_OpenFont("Assets/fonts/arial.ttf", 28);
    try
    {
        _client = std::make_unique<engine::net::UdpSocket>(_ioContext, 0);
        _serverEndpoint = std::make_unique<engine::net::Endpoint>(engine::net::make_endpoint("127.0.0.1", 4242));

        _registry.register_component<component::drawable>();
        _registry.register_component<component::position>();
        _registry.register_component<component::velocity>();
        _registry.register_component<component::controllable>();
        _registry.register_component<component::entity_kind>();
        _registry.register_component<component::collision_state>();
        _registry.register_component<component::animation>();
        _registry.register_component<component::lifetime>();
        _registry.register_component<component::hitbox>();
        _background = std::make_unique<Background>(*this);
        _playerData = std::make_unique<Player>(*this);
        _enemyData = std::make_unique<Enemy>(*this);
        _hud = std::make_unique<Hud>(*this);
        _menu = std::make_unique<R_Type::Menu>(_app);
        _gameOverScreen = std::make_unique<Gameover>(_app);
    }
    catch (const engine::Error &e)
    {
        throw engine::Error(e);
    }
}

R_Type::Rtype::~Rtype() = default;

void R_Type::Rtype::update(float deltaTime,
                           const std::vector<R_Events::Event> &events)
{
    for (auto &ev : events) {
        if (ev.type == R_Events::Type::Quit ||
            (ev.type == R_Events::Type::KeyDown && ev.key.code == R_Events::Key::Escape))
        {
            std::cout << "Quit requested (from gameplay)\n";
            SDL_Quit();
            std::exit(0);
        }
    }
    if (_gameOver)
        return;
    if (_inMenu)
    {
        bool start = _menu->update(events);
        if (start)
        {
            _inMenu = false;
            ConnectReq req{42};
            PacketHeader hdr{CONNECT_REQ, sizeof(ConnectReq), 0};
            std::vector<uint8_t> buf(sizeof(ConnectReq));
            std::memcpy(buf.data(), &req, sizeof(ConnectReq));
            _client->send(hdr, buf, *_serverEndpoint);
            std::cout << "Sent CONNECT_REQ\n";
        }
        return;
    }
    if (!_connected && !_inMenu)
    {
        waiting_connection();
        return;
    }
    
    if (_state == GameState::LOADING)
    {
        _fadeAlpha = std::min(255.0f, _fadeAlpha + (deltaTime * 60.0f));
        return;
    }
    if (_state == GameState::PLAYING && _fadeAlpha > 0)
        _fadeAlpha = std::max(0.0f, _fadeAlpha - (deltaTime * 60.0f));

    auto& profiler = Engine::Profiling::Profiler::getInstance();
    
    for (auto &ev : events)
    {
        if (ev.type == R_Events::Type::KeyDown)
        {
            _pressedKeys.insert(ev.key.code);
            // Toggle profiler overlay with F3
            if (ev.key.code == engine::R_Events::Key::F3) {
                _showProfiler = !_showProfiler;
                if (_profilerOverlay) {
                    _profilerOverlay->setVisible(_showProfiler);
                }
                std::cout << "[Profiling] Overlay " << (_showProfiler ? "shown" : "hidden") << "\n";
            }
        }
        else if (ev.type == R_Events::Type::KeyUp)
            _pressedKeys.erase(ev.key.code);
    }
    static bool wasCPressed = false;
    bool cPressed = _pressedKeys.count(engine::R_Events::Key::C) > 0;
    if (cPressed && !wasCPressed) {
        engine::audio::AudioManager::instance().playSound("projectile");
    }
    wasCPressed = cPressed;
    
    // Toggle debug hitboxes on CTRL+B (either Ctrl key is fine)
    bool ctrlDown = (_pressedKeys.count(engine::R_Events::Key::LCtrl) ||
                     _pressedKeys.count(engine::R_Events::Key::RCtrl));
    bool bDown = (_pressedKeys.count(engine::R_Events::Key::B) > 0);
    static bool prevCombo = false;
    bool combo = ctrlDown && bDown;
    if (combo && !prevCombo)
    {
        _showHitboxes = !_showHitboxes;
    }
    prevCombo = combo;
    
    {
        PROFILE_SCOPE("Network Send");
        InputPacket inp{};
        inp.clientId = _player;
        inp.tick = _tick++;
        inp.keyCount = static_cast<uint16_t>(_pressedKeys.size());
        const uint16_t keyCount = inp.keyCount;
        std::vector<int32_t> keys;
        keys.reserve(keyCount);
        for (auto k : _pressedKeys)
            keys.push_back(static_cast<int32_t>(k));
        const uint16_t payloadSize = sizeof(InputPacket) + keyCount * sizeof(int32_t);
        PacketHeader ihdr{INPUT_PKT, payloadSize, _tick};
        std::vector<uint8_t> ibuf(payloadSize);
        std::memcpy(ibuf.data(), &inp, sizeof(InputPacket));
        if (keyCount > 0)
            std::memcpy(ibuf.data() + sizeof(InputPacket), keys.data(), keyCount * sizeof(int32_t));
        _client->send(ihdr, ibuf, *_serverEndpoint);
    }

    static uint32_t spaceHoldTicks = 0;
    auto shootKeyStr = AccessibilityConfig::key_remap["shoot"];
    auto shootKey = stringToKey(shootKeyStr);
    bool spaceHeld = _pressedKeys.count(shootKey) > 0;
    int numKeys = 0;
    const Uint8 *state = SDL_GetKeyboardState(&numKeys);
    if (state && SDL_SCANCODE_SPACE < numKeys)
    {
        spaceHeld = spaceHeld || (state[SDL_SCANCODE_SPACE] != 0);
    }
    if (spaceHeld)
        spaceHoldTicks++;
    else
        spaceHoldTicks = 0;
    float chargeLevel = std::min(1.0f, spaceHoldTicks / 60.0f);
    if (_hud)
        _hud->setChargeLevel(*this, chargeLevel);
    
    {
        PROFILE_SCOPE("Network Receive");
        receiveSnapshot();
    }
    
    _playerData->playerUpdateAnimation(_entityMap, _player, _registry, _pressedKeys);
    auto &positions = _registry.get_components<component::position>();
    auto &animations = _registry.get_components<component::animation>();
    auto &velocities = _registry.get_components<component::velocity>();
    auto &controls = _registry.get_components<component::controllable>();
    auto &kinds = _registry.get_components<component::entity_kind>();
    auto &drawables = _registry.get_components<component::drawable>();
    auto &collisions = _registry.get_components<component::collision_state>();
    auto &hitboxes = _registry.get_components<component::hitbox>();
    
    {
        PROFILE_SCOPE("Game Systems");
        float adjustedDelta = deltaTime * (AccessibilityConfig::enabled ? AccessibilityConfig::speed_game : 1.0f);
        position_system(_registry, positions, velocities, adjustedDelta);
        control_system(_registry, velocities, controls);
        scroll_reset_system(_registry, positions, kinds, _app);
        animation_system(_registry, animations, drawables, adjustedDelta);
        hitbox_system(_registry, positions, hitboxes, [this](size_t i, size_t j)
                      { this->handle_collision(_registry, i, j); });
        lifetime_system(_registry, adjustedDelta);
        _registry.run_systems();
        _background->update(deltaTime);
    }
    
    // Update world metrics
    auto playerPos = (_player < positions.size() && positions[_player]) 
                     ? positions[_player].value() 
                     : component::position{0, 0};
    profiler.setWorldPosition(playerPos.x, playerPos.y);
    profiler.setEntityCount(_activeEntities.size());
}

void R_Type::Rtype::receiveSnapshot()
{
    if (_state == GameState::LOADING)
    return;
    while (auto pkt_opt = _client->receive(_sender))
    {
        auto [shdr, spayload] = *pkt_opt;

        if (_state == GameState::LOADING && shdr.type == SNAPSHOT)
            continue;

        if (shdr.type == GAME_OVER && spayload.size() >= sizeof(GameOverPayload))
        {
            GameOverPayload go{};
            std::memcpy(&go, spayload.data(), sizeof(go));
            _gameOver = true;
            uint32_t winnerEntityId = go.winnerEntityId;
            _won = (_player == winnerEntityId);
            auto &audio = engine::audio::AudioManager::instance();
            audio.stopMusic();

            continue;
        }
        if (shdr.type == LEVEL_START && spayload.size() >= sizeof(LevelStartPayload))
        {
            LevelStartPayload p{};
            memcpy(&p, spayload.data(), sizeof(LevelStartPayload));
            _state = GameState::PLAYING;
            _fadeAlpha = 255.0f;
            std::cout << "[CLIENT] Leaving LOADING state" << std::endl;
            std::cout << "[CLIENT] LEVEL_START : " << p.level << std::endl;

            _hud->startLevelAnimation(p.level, _registry);
        }
        if (shdr.type == LEVEL_END && spayload.size() >= sizeof(LevelEndPayload))
        {
            LevelEndPayload p{};
            memcpy(&p, spayload.data(), sizeof(LevelEndPayload));
            _state = GameState::LOADING;
            _fadeAlpha = 0.0f;
            _background->changeTheme(p.level + 1);
            std::cout << "[CLIENT] LEVEL_END : " << p.level << std::endl;
            std::cout << "[CLIENT] Entering LOADING state" << std::endl;
        }
        if (shdr.type == SNAPSHOT && spayload.size() >= sizeof(Snapshot))
        {
            Snapshot snap{};
            std::memcpy(&snap, spayload.data(), sizeof(Snapshot));

            auto &positions = _registry.get_components<component::position>();
            auto &velocities = _registry.get_components<component::velocity>();
            auto &drawables = _registry.get_components<component::drawable>();
            auto &kinds = _registry.get_components<component::entity_kind>();
            auto &collisions = _registry.get_components<component::collision_state>();
            auto &animations = _registry.get_components<component::animation>();
            auto &hitboxes = _registry.get_components<component::hitbox>();

            std::unordered_set<uint32_t> newActive;

            size_t n = snap.entityCount;
            if (spayload.size() >= sizeof(Snapshot) + n * sizeof(EntityState))
            {
                auto *entities = reinterpret_cast<const EntityState *>(
                    spayload.data() + sizeof(Snapshot));

                auto ensure_slot = [](auto &arr, std::size_t idx, auto &&value)
                {
                    if (idx >= arr.size())
                    {
                        arr.insert_at(idx, std::forward<decltype(value)>(value));
                    }
                    else if (!arr[idx])
                    {
                        arr.insert_at(idx, std::forward<decltype(value)>(value));
                    }
                };

                auto ensure_cache = [&](size_t idx)
                {
                    if (idx >= _hbW.size())
                    {
                        _hbW.resize(idx + 1, 0.f);
                        _hbH.resize(idx + 1, 0.f);
                        _hbOX.resize(idx + 1, 0.f);
                        _hbOY.resize(idx + 1, 0.f);
                    }
                };

                for (size_t i = 0; i < n; ++i)
                {
                    const EntityState &es = entities[i];

                    size_t idLocal;
                    auto it = _entityMap.find(es.entityId);
                    if (it == _entityMap.end())
                    {
                        idLocal = drawables.size();
                        _entityMap[es.entityId] = idLocal;
                    }
                    else
                    {
                        idLocal = it->second;
                    }
                    auto &hudTags = _registry.get_components<component::hud_tag>();
                    auto &kindsLocal = _registry.get_components<component::entity_kind>();

                    if (idLocal < hudTags.size() && hudTags[idLocal].has_value())
                        continue;
                    if (idLocal < kindsLocal.size()
                        && kindsLocal[idLocal].has_value()
                        && kindsLocal[idLocal].value() == component::entity_kind::decor)
                        continue;
                    ensure_cache(idLocal);
                    newActive.insert(idLocal);

                    ensure_slot(positions, idLocal, component::position{});
                    ensure_slot(velocities, idLocal, component::velocity{});
                    ensure_slot(kinds, idLocal, component::entity_kind{});
                    ensure_slot(collisions, idLocal, component::collision_state{});
                    kinds[idLocal] = static_cast<component::entity_kind>(es.type);
                    ensure_slot(hitboxes, idLocal, component::hitbox{});
                    std::shared_ptr<R_Graphic::Texture> tex;
                    R_Graphic::textureRect rect;
                    component::animation anim;
                    switch (kinds[idLocal].value())
                    {
                    case component::entity_kind::playerProjectile:
                        anim = _playerData->projectileAnimation;
                        tex = _playerData->projectileTexture;
                        rect = _playerData->projectileRect;
                        ensure_slot(hitboxes, idLocal, component::hitbox{100, 24});
                        ensure_slot(drawables, idLocal, component::drawable{tex, rect, layers::Projectiles});
                        break;
                    case component::entity_kind::projectile_charged:
                        anim = _playerData->chargeProjectileAnimation;
                        tex = _playerData->chargeProjectileTexture;
                        rect = _playerData->chargeProjectileRect;
                        ensure_slot(drawables, idLocal, component::drawable{tex, rect, layers::Projectiles});
                        break;
                    case component::entity_kind::projectile_bomb:
                        anim = _playerData->missileProjectileAnimation;
                        tex = _playerData->missileProjectileTexture;
                        rect = _playerData->missileProjectileRect;
                        ensure_slot(drawables, idLocal, component::drawable{tex, rect, layers::Projectiles});
                        break;
                    case component::entity_kind::missile_explosion:
                        anim = _playerData->missileexplosionAnimation;
                        tex = _playerData->missileExplosionTexture;
                        rect = _playerData->missileexplosionRect;
                        ensure_slot(drawables, idLocal, component::drawable{tex, rect, layers::Effects});
                        break;
                    case component::entity_kind::player:
                        anim = _playerData->playerAnimation;
                        tex = _playerData->playerTexture;
                        rect = _playerData->playerRect;
                        ensure_slot(hitboxes, idLocal, component::hitbox{34, 20});
                        if (_playerIndexByLocalId.find(idLocal) == _playerIndexByLocalId.end())
                        {
                            int assigned = static_cast<int>((_playerIndexByLocalId.size() % 5) + 1);
                            _playerIndexByLocalId[idLocal] = assigned;
                        }
                        {
                            const int playerIndex = _playerIndexByLocalId[idLocal];
                            const int rowOffset = (playerIndex - 1) * 17;
                            for (auto &kv : anim.clips)
                            {
                                kv.second.startY = rowOffset;
                            }
                        }
                        ensure_slot(drawables, idLocal, component::drawable{tex, rect, layers::Players});
                        break;
                    case component::entity_kind::enemyProjectile:
                        anim = _enemyData->projectileAnimation;
                        tex = _enemyData->projectileTexture;
                        rect = _enemyData->projectileRect;
                        ensure_slot(hitboxes, idLocal, component::hitbox{60, 60});
                        ensure_slot(drawables, idLocal, component::drawable{tex, rect, layers::Projectiles});
                        break;
                    case component::entity_kind::enemy:
                    {
                        const uint32_t id = es.entityId;

                        if (id % 9 == 0)
                            _enemyData->setType("boss_laser");
                        else if (id % 7 == 0)
                            _enemyData->setType("boss");
                        else if (id % 5 == 0)
                            _enemyData->setType("spinner");
                        else if (id % 4 == 0)
                            _enemyData->setType("charger");
                        else if (id % 3 == 0)
                            _enemyData->setType("shooter");
                        else
                            _enemyData->setType("crawler");

                        tex = _enemyData->enemyTexture;
                        rect = _enemyData->enemyRect;

                        ensure_slot(hitboxes, idLocal, component::hitbox{152, 100});
                        ensure_slot(drawables, idLocal, component::drawable{tex, rect, layers::Enemies});
                        break;
                    }
                    default:
                        tex = _playerData->playerTexture;
                        rect = _playerData->playerRect;
                        ensure_slot(drawables, idLocal, component::drawable{tex, rect, layers::Effects});
                        break;
                    }
                    ensure_slot(animations, idLocal, anim);

                    positions[idLocal]->x = es.x;
                    positions[idLocal]->y = es.y;
                    hitboxes[idLocal]->width = es.hb_w;
                    hitboxes[idLocal]->height = es.hb_h;
                    hitboxes[idLocal]->offset_x = es.hb_ox;
                    hitboxes[idLocal]->offset_y = es.hb_oy;
                    _hbW[idLocal] = es.hb_w;
                    _hbH[idLocal] = es.hb_h;
                    _hbOX[idLocal] = es.hb_ox;
                    _hbOY[idLocal] = es.hb_oy;
                    if (idLocal < kinds.size() && kinds[idLocal] &&
                        kinds[idLocal].value() == component::entity_kind::projectile_bomb)
                    {
                        ensure_slot(animations, idLocal, component::animation{});
                        auto &an = *animations[idLocal];
                        if (es.vy < 0.f)
                            setAnimation(an, "rotation", false);
                        else
                            setAnimation(an, "idle", false);
                    }
                    collisions[idLocal]->collided = (es.collided != 0);
                }
            }

            if (!_activeEntities.empty())
            {
                for (auto id : _activeEntities)
                {
                    if (newActive.find(id) == newActive.end())
                    {
                        if (id == _player)
                            continue;

                        if (id < kinds.size() && kinds[id] &&
                            kinds[id].value() == component::entity_kind::decor)
                        {
                            continue;
                        }

                        if (id < positions.size() && positions[id])
                            positions[id].reset();
                        if (id < velocities.size() && velocities[id])
                            velocities[id].reset();
                        if (id < drawables.size() && drawables[id])
                            drawables[id].reset();
                        if (id < kinds.size() && kinds[id])
                            kinds[id].reset();
                        if (id < collisions.size() && collisions[id])
                            collisions[id].reset();
                        if (id < _hbW.size())
                        {
                            _hbW[id] = 0.f;
                            _hbH[id] = 0.f;
                            _hbOX[id] = 0.f;
                            _hbOY[id] = 0.f;
                        }
                    }
                }
            }
            _activeEntities = std::move(newActive);
        }
    }
}

void R_Type::Rtype::draw()
{
    if (_inMenu) {
        _menu->draw();
        return;
    }
    if (!_connected)
        return;
    if (_gameOver) {
        _fadeAlpha = 0;
        _state = GameState::PLAYING;
        _gameOverScreen->draw(_won);
        return;
    }

    SDL_Renderer* ren = _app.getWindow().getRenderer();
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    auto &positions = _registry.get_components<component::position>();
    auto &drawables = _registry.get_components<component::drawable>();
    auto &kinds = _registry.get_components<component::entity_kind>();
    auto &velocities = _registry.get_components<component::velocity>();

    draw_system(_registry, positions, drawables, _app.getWindow());

    if (_showHitboxes) {
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        auto &hitboxes = _registry.get_components<component::hitbox>();
        hitbox_overlay_system(_registry, positions, hitboxes, kinds, _app.getWindow(), _hitboxOverlayThickness);
    }
    if (_hud)
        _hud->drawOverlay(*this);
    if (AccessibilityConfig::enabled) {
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren, 40, 40, 40, 220);
        SDL_Rect banner = {0, 0, 1920, 80};
        SDL_RenderFillRect(ren, &banner);
        if (_uiFont) {
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface* surf = TTF_RenderUTF8_Blended(_uiFont, "Accessibility mode on", white);
            if (surf) {
                SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
                SDL_Rect dst = {40, 20, surf->w, surf->h};
                SDL_RenderCopy(ren, tex, nullptr, &dst);
                SDL_DestroyTexture(tex);
                SDL_FreeSurface(surf);
            }
        }
    }
    if (_profilerOverlay && _showProfiler)
        _profilerOverlay->render();
    if ((_fadeAlpha > 0.0f || _state == GameState::LOADING) && !_gameOver)
    {
        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren, 0, 0, 0, (Uint8)_fadeAlpha);
        SDL_Rect screen = {0, 0, 1920, 1080};
        SDL_RenderFillRect(ren, &screen);
    }
}

R_Graphic::App &R_Type::Rtype::getApp()
{
    return _app;
}

engine::registry &R_Type::Rtype::getRegistry()
{
    return _registry;
}

void R_Type::Rtype::setServerEndpoint(const std::string &ip, unsigned short port)
{
    _serverEndpoint = std::make_unique<engine::net::Endpoint>(
        engine::net::make_endpoint(ip, port));
}

void R_Type::Rtype::waiting_connection()
{
    if (!_connected)
    {
        if (auto pkt_opt = _client->receive(_sender))
        {
            auto [recvHdr, payload] = *pkt_opt;
            if (recvHdr.type == CONNECT_ACK &&
                payload.size() >= sizeof(ConnectAck))
            {
                ConnectAck ack{};
                std::memcpy(&ack, payload.data(), sizeof(ConnectAck));
                _player = ack.playerEntityId;
                _connected = true;
                _registry.spawn_entity();
            }
        }
    }
}

void R_Type::Rtype::handle_collision(engine::registry &reg, size_t i, size_t j)
{
    auto &positions = reg.get_components<component::position>();
    auto &hitboxes = reg.get_components<component::hitbox>();
    auto &kinds = reg.get_components<component::entity_kind>();
    auto &drawables = reg.get_components<component::drawable>();
    auto &animations = reg.get_components<component::animation>();

    auto kindI = (i < kinds.size() && kinds[i]) ? kinds[i].value() : component::entity_kind::unknown;
    auto kindJ = (j < kinds.size() && kinds[j]) ? kinds[j].value() : component::entity_kind::unknown;

    if ((kindI == component::entity_kind::playerProjectile && kindJ == component::entity_kind::enemy) ||
        (kindJ == component::entity_kind::playerProjectile && kindI == component::entity_kind::enemy))
    {
        engine::audio::AudioManager::instance().playSound("explosion");
        std::cout << "[AUDIO] Explosion sound triggered\n";
        size_t enemyIdx = (kindI == component::entity_kind::enemy) ? i : j;
        float x = positions[enemyIdx]->x;
        float y = positions[enemyIdx]->y;
        auto explosion = reg.spawn_entity();
        reg.add_component(explosion, component::position{x, y});
        reg.add_component(explosion, component::entity_kind::decor);
        component::animation anim = _playerData->explosionAnimation;
        reg.add_component(explosion, component::lifetime{0.8f});
        reg.add_component(explosion, component::drawable{
                                         _playerData->playerTexture,
                                         _playerData->explosionRect,
                                         layers::Effects});
        reg.add_component(explosion, component::animation{anim});
        return;
    }
}

void R_Type::setAnimation(component::animation &anim, const std::string &clip, bool reverse)
{
    if (anim.currentClip != clip && anim.clips.find(clip) != anim.clips.end())
    {
        anim.currentClip = clip;
        anim.currentFrame = 0;
        anim.timer = 0.f;
        anim.reverse = reverse;
    }
}
