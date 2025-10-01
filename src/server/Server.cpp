#include "server/Server.hpp"
#include "engine/ecs/Systems.hpp"
#include "engine/ecs/Systems_client_sfml.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <random>
#include "engine/ecs/EntityFactory.hpp"

namespace {

static void resolve_block(
    std::size_t playerIdx,
    std::size_t wallIdx,
    engine::sparse_array<component::position> &positions,
    engine::sparse_array<component::hitbox> &hitboxes,
    engine::sparse_array<component::collision_state> &collisions,
    engine::sparse_array<component::velocity> &velocities)
{
    if (playerIdx >= collisions.size() || !collisions[playerIdx] || !positions[playerIdx] || !hitboxes[playerIdx] ||
        wallIdx >= positions.size() || !positions[wallIdx] || !hitboxes[wallIdx])
        return;

    collisions[playerIdx]->collided = true;
    auto &playerPos = positions[playerIdx].value();
    const auto &playerHitbox = hitboxes[playerIdx].value();
    const auto &wallPos = positions[wallIdx].value();
    const auto &wallHitbox = hitboxes[wallIdx].value();

    float ax1 = playerPos.x + playerHitbox.offset_x;
    float ay1 = playerPos.y + playerHitbox.offset_y;
    float ax2 = ax1 + playerHitbox.width;
    float ay2 = ay1 + playerHitbox.height;
    float bx1 = wallPos.x + wallHitbox.offset_x;
    float by1 = wallPos.y + wallHitbox.offset_y;
    float bx2 = bx1 + wallHitbox.width;
    float by2 = by1 + wallHitbox.height;

    float overlapX = std::min(ax2, bx2) - std::max(ax1, bx1);
    float overlapY = std::min(ay2, by2) - std::max(ay1, by1);

    if (overlapX <= 0.f || overlapY <= 0.f)
        return;

    float acx = (ax1 + ax2) * 0.5f;
    float bcx = (bx1 + bx2) * 0.5f;
    float acy = (ay1 + ay2) * 0.5f;
    float bcy = (by1 + by2) * 0.5f;

    if (overlapX < overlapY) {
        if (acx < bcx) playerPos.x -= overlapX;
            else playerPos.x += overlapX;
        if (playerIdx < velocities.size() && velocities[playerIdx]) velocities[playerIdx]->vx = 0.f;
    } else {
        if (acy < bcy) playerPos.y -= overlapY;
            else playerPos.y += overlapY;
        if (playerIdx < velocities.size() && velocities[playerIdx]) velocities[playerIdx]->vy = 0.f;
    }
}

static void apply_damage_with_cooldown(
    std::size_t entityIndex,
    uint32_t currentTick,
    engine::registry &reg,
    engine::sparse_array<component::damage> &damages,
    engine::sparse_array<component::damage_cooldown> &cooldowns,
    engine::sparse_array<component::collision_state> &collisions)
{
    if (entityIndex >= damages.size())
        return;

    uint32_t lastHitTick = (entityIndex < cooldowns.size() && cooldowns[entityIndex]) ? cooldowns[entityIndex]->last_hit_tick : 0;
    if (currentTick <= lastHitTick + 60)
        return;

    if (!damages[entityIndex])
        reg.add_component(reg.entity_from_index(entityIndex), component::damage{1});
    else
        damages[entityIndex]->amount += 1;

    if (entityIndex < cooldowns.size() && cooldowns[entityIndex])
        cooldowns[entityIndex]->last_hit_tick = currentTick;
    else
        reg.add_component(reg.entity_from_index(entityIndex), component::damage_cooldown{currentTick});

    if (entityIndex < collisions.size() && collisions[entityIndex])
        collisions[entityIndex]->collided = true;
}

struct SnapshotBuilderContext {
    engine::sparse_array<component::position> &positions;
    engine::sparse_array<component::entity_kind> &kinds;
    engine::sparse_array<component::collision_state> &collisions;
    engine::sparse_array<component::health> &healths;
};

static void try_add_entity(
    uint32_t entityId,
    const std::unordered_set<uint32_t> &alreadyInserted,
    std::vector<EntityState> &out,
    SnapshotBuilderContext &ctx,
    std::unordered_set<uint32_t> &inserted,
    std::size_t limit)
{
    if (out.size() >= limit)
        return;
    if (inserted.find(entityId) != inserted.end())
        return;
    size_t idx = static_cast<size_t>(entityId);
    if (idx >= ctx.positions.size() || !ctx.positions[idx])
        return;

    EntityState es{};
    es.entityId = entityId;
    es.x = ctx.positions[idx]->x;
    es.y = ctx.positions[idx]->y;
    es.vx = 0.f; es.vy = 0.f;
    if (idx < ctx.kinds.size() && ctx.kinds[idx])
        es.type = static_cast<uint8_t>(ctx.kinds[idx].value());
    else
        es.type = static_cast<uint8_t>(component::entity_kind::unknown);
    if (idx < ctx.healths.size() && ctx.healths[idx])
        es.hp = ctx.healths[idx]->hp;
    else
        es.hp = 100;
    es.collided = (idx < ctx.collisions.size() && ctx.collisions[idx] && ctx.collisions[idx]->collided) ? 1 : 0;
    out.push_back(es);
    inserted.insert(entityId);
}

} // namespace

server::server(asio::io_context &ctx, unsigned short port)
    : _socket(ctx, port), _io(ctx), _port(port)
{
    _registry.register_component<component::position>();
    _registry.register_component<component::velocity>();
    _registry.register_component<component::drawable>();
    _registry.register_component<component::hitbox>();
    _registry.register_component<component::controllable>();
    _registry.register_component<component::collision_state>();
    _registry.register_component<component::health>();
    _registry.register_component<component::damage>();
    _registry.register_component<component::despawn_tag>();
    _registry.register_component<component::spawn_request>();
    _registry.register_component<component::entity_kind>();
    _registry.register_component<component::controlled_by>();
    _registry.register_component<component::damage_cooldown>();
    _registry.register_component<component::projectile_tag>();

    (void)_registry.get_components<component::position>();
    (void)_registry.get_components<component::velocity>();
    (void)_registry.get_components<component::drawable>();
    (void)_registry.get_components<component::hitbox>();
    (void)_registry.get_components<component::controllable>();
    (void)_registry.get_components<component::collision_state>();
    (void)_registry.get_components<component::health>();
    (void)_registry.get_components<component::damage>();
    (void)_registry.get_components<component::despawn_tag>();
    (void)_registry.get_components<component::spawn_request>();
    (void)_registry.get_components<component::entity_kind>();
    (void)_registry.get_components<component::controlled_by>();
    (void)_registry.get_components<component::damage_cooldown>();
    (void)_registry.get_components<component::projectile_tag>();

    setup_systems();
}

void server::run()
{
    wait_for_players();
    std::cout << "All players connected!\n";

    using clock = std::chrono::steady_clock;
    const auto tick_duration = std::chrono::milliseconds(16);

    auto last_tick = clock::now();

    while (_running) {
        process_network_inputs();

        auto now = clock::now();
        if (now - last_tick >= tick_duration) {
            game_handler();
            _registry.run_systems();
            broadcast_snapshot();
            _tick++;

            last_tick += tick_duration;

            if (now - last_tick >= tick_duration)
                last_tick = now;
        }
    }
}

void server::stop() { _running = false; }

void server::setup_systems()
{
    _registry.add_system<component::position, component::velocity>(position_system);
    _registry.add_system<component::velocity, component::controllable>(control_system);
    _registry.add_system<component::health, component::despawn_tag, component::damage>(health_system);
    _registry.add_system<component::despawn_tag>([this](engine::registry &reg, engine::sparse_array<component::despawn_tag> &despawns) {
        auto &kinds = _registry.get_components<component::entity_kind>();
        for (auto &&[i, d] : indexed_zipper(despawns)) {
            if (!d.now)
                continue;
            _live_entities.erase(static_cast<uint32_t>(i));
            if (i < kinds.size() && kinds[i] && kinds[i].value() == component::entity_kind::player) {
                auto it = std::find_if(_players.begin(), _players.end(), [i](const PlayerInfo &p) { return static_cast<size_t>(p.entityId) == i; });
                if (it != _players.end())
                    _players.erase(it);
            }
            reg.kill_entity(reg.entity_from_index(i));
        }
    });
    _registry.add_system<component::spawn_request>(spawn_system);

    _registry.add_system<component::position, component::projectile_tag>([this](engine::registry &reg,
        engine::sparse_array<component::position> &positions,
        engine::sparse_array<component::projectile_tag> &projectiles) {
        auto &despawns = reg.get_components<component::despawn_tag>();
        for (auto &&[i, pos, proj] : indexed_zipper(positions, projectiles)) {
            // Move
            pos.x += proj.dirX * proj.speed;
            pos.y += proj.dirY * proj.speed;
            // Lifetime
            if (proj.lifetime > 0)
                --proj.lifetime;
            if (proj.lifetime <= 0) {
                reg.kill_entity(reg.entity_from_index(i));
            }
        }
    });

    _registry.add_system<component::position, component::hitbox>([this](engine::registry &reg,
        engine::sparse_array<component::position> &positions,
        engine::sparse_array<component::hitbox> &hitboxes) {
        auto &collisions = _registry.get_components<component::collision_state>();
        auto &velocities = _registry.get_components<component::velocity>();
        auto &kinds = _registry.get_components<component::entity_kind>();
        auto &damages = _registry.get_components<component::damage>();
        auto &cooldowns = _registry.get_components<component::damage_cooldown>();
        auto &projectiles = _registry.get_components<component::projectile_tag>();
        std::vector<bool> newCollided(collisions.size(), false);

        hitbox_system(reg, positions, hitboxes, [&](std::size_t i, std::size_t j) {
            auto kindI = (i < kinds.size() && kinds[i]) ? kinds[i].value() : component::entity_kind::unknown;
            auto kindJ = (j < kinds.size() && kinds[j]) ? kinds[j].value() : component::entity_kind::unknown;
            if (kindI == component::entity_kind::player && kindJ == component::entity_kind::decor)
                resolve_block(i, j, positions, hitboxes, collisions, velocities);
            if (kindJ == component::entity_kind::player && kindI == component::entity_kind::decor)
                resolve_block(j, i, positions, hitboxes, collisions, velocities);
            if (kindI == component::entity_kind::player && kindJ == component::entity_kind::enemy) {
                apply_damage_with_cooldown(i, _tick, reg, damages, cooldowns, collisions);
                newCollided[i] = true;
            }
            if (kindJ == component::entity_kind::player && kindI == component::entity_kind::enemy) {
                apply_damage_with_cooldown(j, _tick, reg, damages, cooldowns, collisions);
                newCollided[j] = true;
            }



            // Projectile hits enemy or player (friendly fire optional false) -> apply damage and despawn projectile
            if (kindI == component::entity_kind::projectile && kindJ == component::entity_kind::enemy) {
                if (i < projectiles.size() && projectiles[i]) {
                    int dmg = projectiles[i]->damage;
                    if (j < damages.size() && damages[j]) damages[j]->amount += dmg; else reg.add_component(reg.entity_from_index(j), component::damage{dmg});
                    // mark projectile for despawn
                    if (i < _registry.get_components<component::despawn_tag>().size() && _registry.get_components<component::despawn_tag>()[i])
                        _registry.get_components<component::despawn_tag>()[i]->now = true;
                    else
                        reg.add_component(reg.entity_from_index(i), component::despawn_tag{true});
                }
            }
            if (kindJ == component::entity_kind::projectile && kindI == component::entity_kind::enemy) {
                if (j < projectiles.size() && projectiles[j]) {
                    int dmg = projectiles[j]->damage;
                    if (i < damages.size() && damages[i]) damages[i]->amount += dmg; else reg.add_component(reg.entity_from_index(i), component::damage{dmg});
                    if (j < _registry.get_components<component::despawn_tag>().size() && _registry.get_components<component::despawn_tag>()[j])
                        _registry.get_components<component::despawn_tag>()[j]->now = true;
                    else
                        reg.add_component(reg.entity_from_index(j), component::despawn_tag{true});
                }
            }

            

        });
        for (size_t idx = 0; idx < collisions.size(); ++idx)
            if (collisions[idx])
                collisions[idx]->collided = newCollided[idx];
    });
}

void server::wait_for_players()
{
    std::cout << "Waiting for 4 players..." << std::endl;

    while (_players.size() < 1)
    {
        asio::ip::udp::endpoint sender;
        auto pkt_opt = _socket.receive(sender);
        if (pkt_opt)
        {
            auto [hdr, payload] = *pkt_opt;
            if (hdr.type == CONNECT_REQ)
            {
                std::size_t playerIndex = _players.size();
                float spawnX = 100.f;
                float spawnY = 100.f + 120.f * static_cast<float>(playerIndex);

                auto eid = engine::make_entity(
                    _registry,
                    component::position{spawnX, spawnY},
                    component::velocity{0, 0},
                    component::hitbox{40, 40},
                    component::controllable{},
                    component::collision_state{false},
                    component::health{10},
                    component::damage{0},
                    component::entity_kind::player,
                    component::controlled_by{static_cast<uint32_t>(playerIndex)},
                    component::drawable{sf::Vector2f{40.f, 40.f}, sf::Color::Green},
                    component::damage_cooldown{0}
                );
                PlayerInfo pi{sender, eid};
                _live_entities.insert(static_cast<uint32_t>(eid));
                std::cout << "Spawned player entity: " << eid << " for " << sender << "\n";

                _players.push_back(pi);
                ConnectAck ack{1234, 60, static_cast<uint32_t>(eid)};
                PacketHeader h{CONNECT_ACK, static_cast<uint16_t>(sizeof(ConnectAck)), 0};
                std::vector<uint8_t> buf(sizeof(ConnectAck));
                std::memcpy(buf.data(), &ack, sizeof(ConnectAck));
                _socket.send(h, buf, sender);

                broadcast_snapshot();
            }
        }
    }
}

void server::process_network_inputs()
{
    asio::ip::udp::endpoint sender;
    while (auto pkt_opt = _socket.receive(sender)) {
        auto [hdr, payload] = *pkt_opt;
        if (hdr.type == INPUT) {
            if (payload.size() >= sizeof(InputPacket))
            {
                InputPacket input{};
                std::memcpy(&input, payload.data(), sizeof(InputPacket));
                
                if (_live_entities.find(input.clientId) == _live_entities.end())
                    continue;
                
                for (auto &p : _players) {
                    if (p.endpoint == sender && p.entityId == input.clientId) {
                        auto& velocities = _registry.get_components<component::velocity>();
                        if (static_cast<size_t>(p.entityId) < velocities.size() && velocities[p.entityId]) {
                            auto &vel = *velocities[p.entityId];
                            vel.vx = (input.keys & 0x01) ? -10 : (input.keys & 0x02) ? 10 : 0;
                            vel.vy = (input.keys & 0x04) ? -10 : (input.keys & 0x08) ? 10 : 0;
                        }
                        break;
                    }
                }
            }
        }
    }
}

void server::game_handler()
{
    auto &positions = _registry.get_components<component::position>();
    int live_entity_count = 0;

    for (size_t i = 0; i < positions.size(); ++i)
        if (positions[i])
            live_entity_count++;

/*     if (_tick % 1 == 0 && live_entity_count < 60) {
        int posX = std::uniform_int_distribution<int>(100, 1820)(_gen);
        int posY = std::uniform_int_distribution<int>(100, 980)(_gen);
        auto enemy = engine::make_entity(
            _registry,
            component::position{(float)posX, (float)posY},
            component::velocity{0.f, 0.f},
            component::hitbox{40.f, 40.f},
            component::entity_kind::decor,
            component::collision_state{false},
            component::health{1}
        );
        _live_entities.insert(static_cast<uint32_t>(enemy));
    } */

    // Test projectile spawning: every 30 ticks each player shoots one projectile to the right
    if (_tick % 30 == 0) {
        auto &kinds = _registry.get_components<component::entity_kind>();
        auto &hitboxes = _registry.get_components<component::hitbox>();
        for (auto &p : _players) {
            size_t idx = static_cast<size_t>(p.entityId);
            if (idx >= positions.size() || !positions[idx]) continue;
            auto pos = positions[idx].value();
            // Compute front spawn using player's hitbox (if any) to avoid overlapping the player box
            float playerW = 0.f, playerH = 0.f;
            if (idx < hitboxes.size() && hitboxes[idx]) {
                playerW = hitboxes[idx]->width;
                playerH = hitboxes[idx]->height;
            }
            constexpr float projectileW = 10.f;
            constexpr float projectileH = 10.f;
            float startX = pos.x + playerW + 4.f; // 4px margin in front
            float startY = pos.y + (playerH * 0.5f) - (projectileH * 0.5f); // center vertically
            auto proj = engine::make_entity(
                _registry,
                component::position{startX, startY},
                component::hitbox{projectileW, projectileH},
                component::collision_state{false},
                component::entity_kind::projectile,
                component::projectile_tag{static_cast<uint32_t>(p.entityId), 4, 1.f, 0.f, 25.f, 2},
                component::health{1}
            );
            _live_entities.insert(static_cast<uint32_t>(proj));
        }
    }
}

void server::broadcast_snapshot()
{
    auto &positions = _registry.get_components<component::position>();
    auto &kinds = _registry.get_components<component::entity_kind>();
    auto &collisions = _registry.get_components<component::collision_state>();
    auto &healths = _registry.get_components<component::health>();
    auto &despawns = _registry.get_components<component::despawn_tag>();

    constexpr std::size_t SNAPSHOT_LIMIT = 40;
    std::vector<EntityState> states; states.reserve(50);
    std::unordered_set<uint32_t> inserted; // track inside this call

    SnapshotBuilderContext ctx{positions, kinds, collisions, healths};

    for (auto &pInfo : _players) {
        size_t idx = static_cast<size_t>(pInfo.entityId);
        if (idx < despawns.size() && despawns[idx] && despawns[idx]->now) continue; // skip about-to-despawn
        try_add_entity(static_cast<uint32_t>(pInfo.entityId), inserted, states, ctx, inserted, SNAPSHOT_LIMIT);
    }
    for (uint32_t entityId : _live_entities) {
        if (states.size() >= SNAPSHOT_LIMIT) break;
        size_t idx = static_cast<size_t>(entityId);
        if (idx < despawns.size() && despawns[idx] && despawns[idx]->now) continue;
        try_add_entity(entityId, inserted, states, ctx, inserted, SNAPSHOT_LIMIT);
    }
    if (states.empty())
        return;

    Snapshot snap{_tick, static_cast<uint16_t>(states.size())};
    std::vector<uint8_t> buf(sizeof(Snapshot) + sizeof(EntityState) * states.size());
    std::memcpy(buf.data(), &snap, sizeof(Snapshot));
    std::memcpy(buf.data() + sizeof(Snapshot), states.data(), sizeof(EntityState) * states.size());
    PacketHeader hdr{SNAPSHOT, static_cast<uint16_t>(buf.size()), _tick};
    for (auto &p : _players)
        _socket.send(hdr, buf, p.endpoint);

}