#include "server/Server.hpp"
#include "engine/ecs/Systems.hpp"
#include "engine/ecs/Systems_client_sfml.hpp"
#include <thread>
#include <chrono>
#include <iostream>
#include <random>
#include "engine/ecs/EntityFactory.hpp"

namespace {

// ---------- Collision helpers ----------
static void resolve_block(
    std::size_t p,
    std::size_t wall,
    engine::sparse_array<component::position> &positions,
    engine::sparse_array<component::hitbox> &hitboxes,
    engine::sparse_array<component::collision_state> &collisions,
    engine::sparse_array<component::velocity> &velocities)
{
    if (p >= collisions.size() || !collisions[p] || !positions[p] || !hitboxes[p] || wall >= positions.size() || !positions[wall] || !hitboxes[wall])
        return;
    collisions[p]->collided = true;
    auto &ppos = positions[p].value();
    const auto &phb = hitboxes[p].value();
    const auto &opos = positions[wall].value();
    const auto &ohb = hitboxes[wall].value();

    float ax1 = ppos.x + phb.offset_x;
    float ay1 = ppos.y + phb.offset_y;
    float ax2 = ax1 + phb.width;
    float ay2 = ay1 + phb.height;
    float bx1 = opos.x + ohb.offset_x;
    float by1 = opos.y + ohb.offset_y;
    float bx2 = bx1 + ohb.width;
    float by2 = by1 + ohb.height;

    float overlapX = std::min(ax2, bx2) - std::max(ax1, bx1);
    float overlapY = std::min(ay2, by2) - std::max(ay1, by1);

    if (overlapX <= 0.f || overlapY <= 0.f)
        return;

    float acx = (ax1 + ax2) * 0.5f;
    float bcx = (bx1 + bx2) * 0.5f;
    float acy = (ay1 + ay2) * 0.5f;
    float bcy = (by1 + by2) * 0.5f;

    if (overlapX < overlapY) {
        if (acx < bcx) ppos.x -= overlapX; else ppos.x += overlapX;
        if (p < velocities.size() && velocities[p]) velocities[p]->vx = 0.f;
    } else {
        if (acy < bcy) ppos.y -= overlapY; else ppos.y += overlapY;
        if (p < velocities.size() && velocities[p]) velocities[p]->vy = 0.f;
    }
}

static void apply_damage_with_cooldown(
    std::size_t p,
    uint32_t currentTick,
    engine::registry &reg,
    engine::sparse_array<component::damage> &damages,
    engine::sparse_array<component::damage_cooldown> &cooldowns,
    engine::sparse_array<component::collision_state> &collisions)
{
    if (p >= damages.size()) return;
    uint32_t last = (p < cooldowns.size() && cooldowns[p]) ? cooldowns[p]->last_hit_tick : 0;
    if (currentTick <= last + 60) return;

    if (!damages[p])
        reg.add_component(reg.entity_from_index(p), component::damage{1});
    else
        damages[p]->amount += 1;

    if (p < cooldowns.size() && cooldowns[p])
        cooldowns[p]->last_hit_tick = currentTick;
    else
        reg.add_component(reg.entity_from_index(p), component::damage_cooldown{currentTick});

    if (p < collisions.size() && collisions[p])
        collisions[p]->collided = true;
}

// ---------- Snapshot build helper ----------
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
    if (out.size() >= limit) return;
    if (inserted.find(entityId) != inserted.end()) return;
    size_t idx = static_cast<size_t>(entityId);
    if (idx >= ctx.positions.size() || !ctx.positions[idx]) return;

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

} // anonymous namespace

server::server(asio::io_context &ctx, unsigned short port)
    : _socket(ctx, port), _io(ctx), _port(port)
{
    // Component registration
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

    // Access arrays once to ensure they are created (optional but keeps parity with old code)
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

    setup_systems();
}

void server::run()
{
    wait_for_players();
    std::cout << "All players connected!\n";

    using clock = std::chrono::steady_clock;
    const auto tick_duration = std::chrono::milliseconds(16);

    auto last_tick = clock::now();

    while (_running)
    {
        // Always process inputs as fast as possible
        process_network_inputs();

        auto now = clock::now();
        if (now - last_tick >= tick_duration)
        {
            // Only advance game state if 16ms passed
            game_handler();
            _registry.run_systems();
            broadcast_snapshot();
            _tick++;

            last_tick += tick_duration;

            // Optional catch-up if the server is late
            if (now - last_tick >= tick_duration) {
                // we fell behind, resync to current time
                last_tick = now;
            }
        }

        // No sleep here! Loop just runs fast and keeps receiving inputs.
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
            if (!d.now) continue;
            _live_entities.erase(static_cast<uint32_t>(i));
            if (i < kinds.size() && kinds[i] && kinds[i].value() == component::entity_kind::player) {
                auto it = std::find_if(_players.begin(), _players.end(), [i](const PlayerInfo &p) { return static_cast<size_t>(p.entityId) == i; });
                if (it != _players.end()) _players.erase(it);
            }
            reg.kill_entity(reg.entity_from_index(i));
        }
    });
    _registry.add_system<component::spawn_request>(spawn_system);

    _registry.add_system<component::position, component::hitbox>([this](engine::registry &reg,
        engine::sparse_array<component::position> &positions,
        engine::sparse_array<component::hitbox> &hitboxes) {
        auto &collisions = _registry.get_components<component::collision_state>();
        auto &velocities = _registry.get_components<component::velocity>();
        auto &kinds = _registry.get_components<component::entity_kind>();
        auto &damages = _registry.get_components<component::damage>();
        auto &cooldowns = _registry.get_components<component::damage_cooldown>();
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
        });
        for (size_t idx = 0; idx < collisions.size(); ++idx)
            if (collisions[idx])
                collisions[idx]->collided = newCollided[idx];
    });
}

void server::wait_for_players()
{
    std::cout << "Waiting for 4 players..." << std::endl;

    while (_players.size() < 2)
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
                PacketHeader h{CONNECT_ACK,
                               static_cast<uint16_t>(sizeof(ConnectAck)),
                               0};
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
    while (auto pkt_opt = _socket.receive(sender))
    {
        auto [hdr, payload] = *pkt_opt;
        if (hdr.type == INPUT)
        {
            if (payload.size() >= sizeof(InputPacket))
            {
                InputPacket input{};
                std::memcpy(&input, payload.data(), sizeof(InputPacket));
                
                if (_live_entities.find(input.clientId) == _live_entities.end()) {
                    continue;
                }
                
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
    {
        if (positions[i])
        {
            live_entity_count++;
        }
    }

    if (_tick % 1 == 0 && live_entity_count < 60)
    { // Much lower limit
        int posX = std::uniform_int_distribution<int>(100, 1820)(_gen);
        int posY = std::uniform_int_distribution<int>(100, 980)(_gen);
        auto enemy = engine::make_entity(
            _registry,
            component::position{(float)posX, (float)posY},
            component::velocity{0.f, 0.f},
            component::hitbox{40.f, 40.f},
            component::entity_kind::enemy,
            component::collision_state{false},
            component::health{1}
        );
        _live_entities.insert(static_cast<uint32_t>(enemy));
    }
}

void server::broadcast_snapshot()
{
    auto &positions = _registry.get_components<component::position>();
    auto &kinds = _registry.get_components<component::entity_kind>();
    auto &collisions = _registry.get_components<component::collision_state>();
    auto &healths = _registry.get_components<component::health>();

    constexpr std::size_t SNAPSHOT_LIMIT = 40;
    std::vector<EntityState> states; states.reserve(50);
    std::unordered_set<uint32_t> inserted; // track inside this call

    SnapshotBuilderContext ctx{positions, kinds, collisions, healths};

    for (auto &pInfo : _players) try_add_entity(static_cast<uint32_t>(pInfo.entityId), inserted, states, ctx, inserted, SNAPSHOT_LIMIT);
    for (uint32_t entityId : _live_entities) {
        if (states.size() >= SNAPSHOT_LIMIT) break;
        try_add_entity(entityId, inserted, states, ctx, inserted, SNAPSHOT_LIMIT);
    }
    if (states.empty()) return;

    Snapshot snap{_tick, static_cast<uint16_t>(states.size())};
    std::vector<uint8_t> buf(sizeof(Snapshot) + sizeof(EntityState) * states.size());
    std::memcpy(buf.data(), &snap, sizeof(Snapshot));
    std::memcpy(buf.data() + sizeof(Snapshot), states.data(), sizeof(EntityState) * states.size());
    PacketHeader hdr{SNAPSHOT, static_cast<uint16_t>(buf.size()), _tick};
    for (auto &p : _players) _socket.send(hdr, buf, p.endpoint);
}