
/**
 * @file Server.cpp
 * @brief Implementation of the main server logic for the R-Type game.
 *
 * This file contains the core server-side logic, including entity management,
 * network communication, game tick handling, system registration, and enemy spawning.
 * The server uses an ECS (Entity Component System) architecture to manage game entities
 * and their behaviors, and communicates with clients via UDP sockets.
 *
 * Main functionalities:
 * - Registers and manages game components and systems.
 * - Handles player connections and spawns player entities.
 * - Processes network inputs from clients to update player states.
 * - Spawns enemies and projectiles at regular intervals based on game ticks.
 * - Runs game logic and updates entity states each tick.
 * - Broadcasts game state snapshots to all connected players.
 *
 * Key classes and functions:
 * - server::server: Constructor, initializes server and registers components/systems.
 * - server::run: Main game loop, processes inputs, updates game state, and broadcasts snapshots.
 * - server::register_components: Registers all ECS components used in the game.
 * - server::setup_systems: Registers all ECS systems, including AI and collision handling.
 * - server::game_handler: Spawns enemies and handles game-specific logic per tick.
 * - server::broadcast_snapshot: Sends the current game state to all players.
 * - server::wait_for_players: Waits for player connections and spawns player entities.
 * - server::process_network_inputs: Handles incoming network packets and updates player states.
 * - server::spawn_player: Spawns a new player entity with default components.
 * - server::spawn_projectile: Spawns a projectile entity for a given owner.
 *
 */
#include "server/Server.hpp"
#include "common/Systems.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "server/ServerUtils.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <random>
#include <thread>

#include "common/Components_client.hpp"
#include "common/Systems.hpp"
#include "server/Components_ai.hpp"
#include "server/EnemyConfig.hpp"
#include "server/Server.hpp"
#include "server/System_ai.hpp"

using json = nlohmann::json;

using namespace serverutils;

server::server(engine::net::IoContext &ctx, unsigned short port)
    : _socket(ctx, port), _io(ctx), _port(port)
{
  register_components();
}

// Default components
void server::register_components()
{
  _registry.register_component<component::position>();
  _registry.register_component<component::velocity>();
  _registry.register_component<component::hitbox>();
  _registry.register_component<component::controllable>();
  _registry.register_component<component::collision_state>();
  _registry.register_component<component::health>();
  _registry.register_component<component::damage>();
  _registry.register_component<component::spawn_request>();
  _registry.register_component<component::entity_kind>();
  _registry.register_component<component::controlled_by>();
  _registry.register_component<component::damage_cooldown>();
  _registry.register_component<component::projectile_tag>();
  // New modular components
  _registry.register_component<component::gravity>();
  _registry.register_component<component::area_effect>();

  systems::init_ai_behaviors();
  _registry.add_system<component::position, component::velocity,
                       component::ai_controller>(
      [this](auto &reg, auto &pos, auto &vel, auto &ai)
      {
        systems::enemy_ai_system(reg, pos, vel, ai, _tick);
      });
  _registry.register_component<component::ai_controller>();
  _registry.register_component<component::spell>();
  _registry.register_component<component::spellbook>();
  _registry.register_component<component::boss_phase>();

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
    process_network_inputs();

        auto now = clock::now();
        if (now - last_tick >= tick_duration)
        {
            auto &positions = _registry.get_components<component::position>();
            auto &velocities = _registry.get_components<component::velocity>();
            auto &controls = _registry.get_components<component::controllable>();
            game_handler();
            position_system(_registry, positions, velocities, 1.0f / 60.0f);
            _registry.run_systems();
            broadcast_snapshot();
            _tick++;

      last_tick += tick_duration;

      if (now - last_tick >= tick_duration)
      {
        last_tick = now;
      }
    }
  }
}

void server::stop() { _running = false; }

void server::setup_systems()
{
  register_health_and_spawn_systems();
  register_projectile_movement_system();
  register_gravity_system();
  register_collision_system();
  register_bounds_system();
  register_area_effect_system();
}

void server::register_health_and_spawn_systems()
{
  _registry.add_system<component::health, component::damage>(health_system);
  _registry.add_system<component::spawn_request>(spawn_system);
}

void server::register_projectile_movement_system()
{
  _registry.add_system<component::position, component::projectile_tag>(
      [this](engine::registry &reg,
             engine::sparse_array<component::position> &positions,
             engine::sparse_array<component::projectile_tag> &projectiles) {
        std::vector<engine::entity_t> toKill;
        for (auto &&[i, pos, proj] : indexed_zipper(positions, projectiles))
        {
          pos.x += proj.dirX * proj.speed;
          pos.y += proj.dirY * proj.speed;
          if (proj.lifetime > 0)
            --proj.lifetime;
          if (proj.lifetime <= 0)
            toKill.push_back(reg.entity_from_index(i));
        }
        for (auto e : toKill)
        {
          _live_entities.erase(static_cast<uint32_t>(e));
          reg.kill_entity(e);
        }
      });
}

void server::register_gravity_system()
{
  _registry.add_system<component::projectile_tag, component::gravity, component::velocity>(
      [this](engine::registry &reg,
             engine::sparse_array<component::projectile_tag> &projectiles,
             engine::sparse_array<component::gravity> &gravs,
             engine::sparse_array<component::velocity> &vels) {
        for (auto &&[i, proj, g, vel] : indexed_zipper(projectiles, gravs, vels))
        {
          (void)i;
          proj.dirY += g.ay;
          vel.vx = proj.dirX * proj.speed;
          vel.vy = proj.dirY * proj.speed;
        }
      });
}

void server::register_collision_system()
{
  _registry.add_system<component::position, component::hitbox>(
      [this](engine::registry &reg,
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

          if (kindI == component::entity_kind::player && kindJ == component::entity_kind::enemy)
          {
            apply_damage_with_cooldown(i, _tick, reg, damages, cooldowns, collisions);
            newCollided[i] = true;
            resolve_block(i, j, positions, hitboxes, collisions, velocities);
          }
          if (kindJ == component::entity_kind::player && kindI == component::entity_kind::enemy)
          {
            apply_damage_with_cooldown(j, _tick, reg, damages, cooldowns, collisions);
            newCollided[j] = true;
            resolve_block(j, i, positions, hitboxes, collisions, velocities);
          }

          if ((kindI == component::entity_kind::projectile || kindI == component::entity_kind::projectile_bomb || kindI == component::entity_kind::projectile_charged) &&
              kindJ == component::entity_kind::enemy)
          {
            if (i < projectiles.size() && projectiles[i])
            {
              auto &proj = projectiles[i].value();
              auto ownerKind = (proj.owner < kinds.size() && kinds[proj.owner]) ? kinds[proj.owner].value() : component::entity_kind::unknown;
              if (ownerKind != component::entity_kind::enemy)
              {
                if (j < damages.size() && damages[j]) damages[j]->amount += proj.damage;
                else reg.add_component(reg.entity_from_index(j), component::damage{proj.damage});
                if (kindI == component::entity_kind::projectile_bomb)
                {
                  auto &posArr = _registry.get_components<component::position>();
                  if (i < posArr.size() && posArr[i])
                  {
                    auto pPos = posArr[i].value();
                    auto exp = spawn_missile_explosion(pPos.x, pPos.y, proj.damage, 180.f);
                    _live_entities.insert(static_cast<uint32_t>(exp));
                  }
                }
                _live_entities.erase(static_cast<uint32_t>(reg.entity_from_index(i)));
                reg.kill_entity(reg.entity_from_index(i));
              }
            }
          }

          if ((kindJ == component::entity_kind::projectile || kindJ == component::entity_kind::projectile_charged || kindJ == component::entity_kind::projectile_bomb) &&
              kindI == component::entity_kind::enemy)
          {
            if (j < projectiles.size() && projectiles[j])
            {
              auto &proj = projectiles[j].value();
              auto ownerKind = (proj.owner < kinds.size() && kinds[proj.owner]) ? kinds[proj.owner].value() : component::entity_kind::unknown;
              if (ownerKind != component::entity_kind::enemy)
              {
                if (i < damages.size() && damages[i]) damages[i]->amount += proj.damage;
                else reg.add_component(reg.entity_from_index(i), component::damage{proj.damage});
                if (kindJ == component::entity_kind::projectile_bomb)
                {
                  auto &posArr = _registry.get_components<component::position>();
                  if (j < posArr.size() && posArr[j])
                  {
                    auto pPos = posArr[j].value();
                    auto exp = spawn_missile_explosion(pPos.x, pPos.y, proj.damage, 180.f);
                    _live_entities.insert(static_cast<uint32_t>(exp));
                  }
                }
                _live_entities.erase(static_cast<uint32_t>(reg.entity_from_index(j)));
                reg.kill_entity(reg.entity_from_index(j));
              }
            }
          }

          if ((kindI == component::entity_kind::projectile || kindI == component::entity_kind::projectile_bomb) && kindJ == component::entity_kind::player)
          {
            if (i < projectiles.size() && projectiles[i])
            {
              auto &proj = projectiles[i].value();
              auto ownerKind = (proj.owner < kinds.size() && kinds[proj.owner]) ? kinds[proj.owner].value() : component::entity_kind::unknown;
              if (ownerKind != component::entity_kind::player)
              {
                if (j < damages.size() && damages[j]) damages[j]->amount += proj.damage;
                else reg.add_component(reg.entity_from_index(j), component::damage{proj.damage});
                _live_entities.erase(static_cast<uint32_t>(reg.entity_from_index(i)));
                reg.kill_entity(reg.entity_from_index(i));
              }
            }
          }

          if ((kindJ == component::entity_kind::projectile || kindJ == component::entity_kind::projectile_bomb) && kindI == component::entity_kind::player)
          {
            if (j < projectiles.size() && projectiles[j])
            {
              auto &proj = projectiles[j].value();
              auto ownerKind = (proj.owner < kinds.size() && kinds[proj.owner]) ? kinds[proj.owner].value() : component::entity_kind::unknown;
              if (ownerKind != component::entity_kind::player)
              {
                if (i < damages.size() && damages[i]) damages[i]->amount += proj.damage;
                else reg.add_component(reg.entity_from_index(i), component::damage{proj.damage});
                _live_entities.erase(static_cast<uint32_t>(reg.entity_from_index(j)));
                reg.kill_entity(reg.entity_from_index(j));
              }
            }
          }
        });

        for (std::size_t idx = 0; idx < collisions.size(); ++idx)
        {
          if (collisions[idx]) collisions[idx]->collided = newCollided[idx];
        }
      });
}

void server::register_bounds_system()
{
  constexpr float SCREEN_WIDTH = 1920.f;
  constexpr float SCREEN_HEIGHT = 1080.f;
  _registry.add_system<component::position, component::velocity, component::entity_kind>(
      [this](engine::registry &reg,
             engine::sparse_array<component::position> &positions,
             engine::sparse_array<component::velocity> &velocities,
             engine::sparse_array<component::entity_kind> &kinds) {
        std::vector<engine::entity_t> toKill;
        for (auto &&[i, pos, vel, kind] : engine::indexed_zipper(positions, velocities, kinds))
        {
          float x = pos.x;
          float y = pos.y;

          if (kind == component::entity_kind::projectile)
          {
            if (x < -50.f || x > SCREEN_WIDTH + 50.f || y < -50.f || y > SCREEN_HEIGHT + 50.f)
            {
              toKill.push_back(reg.entity_from_index(i));
            }
            continue;
          }

          bool corrected = false;
          if (x < 0.f) { x = 0.f; corrected = true; }
          else if (x > SCREEN_WIDTH) { x = SCREEN_WIDTH; corrected = true; }
          if (y < 0.f) { y = 0.f; corrected = true; }
          else if (y > SCREEN_HEIGHT) { y = SCREEN_HEIGHT; corrected = true; }
          if (corrected)
          {
            pos.x = x;
            pos.y = y;
            vel.vx = (x <= 0.f || x >= SCREEN_WIDTH) ? 0.f : vel.vx;
            vel.vy = (y <= 0.f || y >= SCREEN_HEIGHT) ? 0.f : vel.vy;
          }
        }

        for (auto e : toKill)
        {
          _live_entities.erase(static_cast<uint32_t>(e));
          reg.kill_entity(e);
        }
      });
}

void server::register_area_effect_system()
{
  _registry.add_system<component::position, component::area_effect, component::entity_kind>(
      [this](engine::registry &reg,
             engine::sparse_array<component::position> &positions,
             engine::sparse_array<component::area_effect> &areas,
             engine::sparse_array<component::entity_kind> &kinds) {
        auto &damages = _registry.get_components<component::damage>();
        for (auto &&[i, pos, area, kind] : indexed_zipper(positions, areas, kinds))
        {
          (void)i;
          if (kind != component::entity_kind::missile_explosion) continue;
          if (area.applied) continue;
          auto &kindsArr = _registry.get_components<component::entity_kind>();
          auto &posArr = _registry.get_components<component::position>();
          for (size_t j = 0; j < kindsArr.size(); ++j)
          {
            if (j >= posArr.size() || !kindsArr[j] || !posArr[j]) continue;
            if (kindsArr[j].value() != component::entity_kind::enemy) continue;
            float centerX = pos.x + (area.radius);
            float centerY = pos.y + (area.radius);
            auto ep = posArr[j].value();
            float dx = ep.x - centerX;
            float dy = ep.y - centerY;
            if ((dx * dx + dy * dy) <= area.radius * area.radius)
            {
              if (j < damages.size() && damages[j]) damages[j]->amount += area.damage;
              else reg.add_component(reg.entity_from_index(j), component::damage{area.damage});
            }
          }
          area.applied = true;
        }
      });
}

void server::game_handler()
{
  for (auto &p : _players)
  {
    _live_entities.insert(static_cast<uint32_t>(p.entityId));
  }
  for (auto e : systems::spawned_projectiles)
  {
    _live_entities.insert(static_cast<uint32_t>(e));
  }
  systems::spawned_projectiles.clear();
    if (_tick % 200 == 0)
    {
        try
        {
            EnemyConfig cfg = EnemyConfig::load_enemy_config("configs/enemy/crawler.json");
            auto e = _registry.spawn_entity();
            _live_entities.insert((uint32_t)e);

      _registry.add_component(e, component::position{1820, static_cast<float>(std::uniform_int_distribution<int>(100, 1000)(_gen))});
      _registry.add_component(e, component::velocity{0, 0});
      _registry.add_component<component::hitbox>(e, std::move(cfg.hitbox));
      _registry.add_component(e, component::entity_kind::enemy);
      _registry.add_component(e, component::collision_state{false});
      _registry.add_component(e, component::health{(uint8_t)cfg.hp});

      component::ai_controller ai;
      ai.behavior = cfg.behavior;
      ai.speed = cfg.speed;
      _registry.add_component<component::ai_controller>(e, std::move(ai));
      if (!cfg.spells.empty())
      {
        component::spellbook sb;
        sb.spells = cfg.spells;
        _registry.add_component<component::spellbook>(e, std::move(sb));
      }
    }
    catch (std::exception &ex)
    {
      std::cerr << "Failed to load zigzag enemy: " << ex.what() << "\n";
    }
  }

    if (_tick % 400 == 0)
    {
        try
        {
            EnemyConfig cfg = EnemyConfig::load_enemy_config("configs/enemy/shooter.json");
            auto e = _registry.spawn_entity();
            _live_entities.insert((uint32_t)e);

      _registry.add_component(e, component::position{1820, static_cast<float>(std::uniform_int_distribution<int>(100, 1000)(_gen))});
      _registry.add_component(e, component::velocity{0, 0});
      _registry.add_component<component::hitbox>(e, std::move(cfg.hitbox));
      _registry.add_component(e, component::entity_kind::enemy);
      _registry.add_component(e, component::collision_state{false});
      _registry.add_component(e, component::health{(uint8_t)cfg.hp});
      component::ai_controller ai;
      ai.behavior = cfg.behavior;
      ai.speed = cfg.speed;
      _registry.add_component<component::ai_controller>(e, std::move(ai));
      std::cout << "Enemy spawned with behavior=" << ai.behavior
                << " speed=" << ai.speed << "\n";
      if (!cfg.spells.empty())
      {
        component::spellbook sb;
        sb.spells = cfg.spells;
        _registry.add_component<component::spellbook>(e, std::move(sb));
      }
      std::cout << "Spawned Shooter enemy\n";
    }
    catch (std::exception &ex)
    {
      std::cerr << "Failed to load shooter enemy: " << ex.what() << "\n";
    }
  }
  if (_tick == 600)
  {
    try
    {
      EnemyConfig cfg = EnemyConfig::load_enemy_config("configs/enemy/boss.json");
      auto boss = _registry.spawn_entity();
      _live_entities.insert(static_cast<uint32_t>(boss));

      _registry.add_component(boss, component::position{1800.f, 400.f});
      _registry.add_component(boss, component::velocity{0.f, 0.f});
      _registry.add_component<component::hitbox>(boss, std::move(cfg.hitbox));
      _registry.add_component(boss, component::entity_kind::enemy);
      _registry.add_component(boss, component::collision_state{false});
      _registry.add_component(boss, component::health{(uint8_t)cfg.hp});

      component::ai_controller ai;
      ai.behavior = cfg.behavior; // "boss"
      ai.speed = cfg.speed;
      _registry.add_component<component::ai_controller>(boss, std::move(ai));

      if (!cfg.spells.empty())
      {
          component::spellbook sb;
          sb.spells = cfg.spells;
          _registry.add_component<component::spellbook>(boss, std::move(sb));
      }

      std::cout << "Boss spawned! HP: " << cfg.hp << " Behavior: " << ai.behavior << "\n";
    }
    catch (std::exception &ex)
    {
      std::cerr << "Failed to spawn boss: " << ex.what() << "\n";
    }
  }
}

void server::broadcast_snapshot()
{
  auto &positions = _registry.get_components<component::position>();
  auto &kinds = _registry.get_components<component::entity_kind>();
  auto &collisions = _registry.get_components<component::collision_state>();
  auto &healths = _registry.get_components<component::health>();
  auto &velocities = _registry.get_components<component::velocity>();

  constexpr std::size_t SNAPSHOT_LIMIT = 10000;
  std::vector<EntityState> states;
  states.reserve(50);
  std::unordered_set<uint32_t> inserted;

  auto &hitboxes = _registry.get_components<component::hitbox>();
  SnapshotBuilderContext ctx{positions, velocities, kinds, collisions, healths, hitboxes};
  for (auto &pInfo : _players)
  {
    try_add_entity(static_cast<uint32_t>(pInfo.entityId), states, ctx, inserted,
                   SNAPSHOT_LIMIT);
  }
  for (uint32_t entityId : _live_entities)
  {

    if (states.size() >= SNAPSHOT_LIMIT)
      break;
    size_t idx = static_cast<size_t>(entityId);
    try_add_entity(entityId, states, ctx, inserted, SNAPSHOT_LIMIT);
  }
  if (states.empty())
    return;

  Snapshot snap{_tick, static_cast<uint16_t>(states.size())};
  std::vector<uint8_t> buf(sizeof(Snapshot) +
                           sizeof(EntityState) * states.size());
  std::memcpy(buf.data(), &snap, sizeof(Snapshot));
  std::memcpy(buf.data() + sizeof(Snapshot), states.data(),
              sizeof(EntityState) * states.size());
  PacketHeader hdr{SNAPSHOT, static_cast<uint16_t>(buf.size()), _tick};
  for (auto &p : _players)
    _socket.send(hdr, buf, p.endpoint);
}

void server::wait_for_players()
{
  std::cout << "Waiting for 4 players..." << std::endl;

    while (_players.size() < 1) {
    engine::net::Endpoint sender;
        auto pkt_opt = _socket.receive(sender);
        if (pkt_opt)
        {
            auto [hdr, payload] = *pkt_opt;
            if (hdr.type == CONNECT_REQ)
            {
                std::size_t playerIndex = _players.size();
                float spawnX = 100.f;
                float spawnY = 100.f + 120.f * static_cast<float>(playerIndex);

        auto eid = spawn_player(sender, playerIndex);
        PlayerInfo pi{sender, eid};
        _live_entities.insert(static_cast<uint32_t>(eid));
        std::cout << "Spawned player entity: " << eid << " for " << sender.address << ":" << sender.port
                  << "\n";

        _players.push_back(pi);
        ConnectAck ack{1234, 60, static_cast<uint16_t>(eid)};
        PacketHeader h{CONNECT_ACK, static_cast<uint16_t>(sizeof(ConnectAck)),
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
  engine::net::Endpoint sender;
  while (auto pkt_opt = _socket.receive(sender))
  {
    auto [hdr, payload] = *pkt_opt;
    if (hdr.type == INPUT_PKT)
    {
      if (payload.size() >= sizeof(InputPacket))
      {
        InputPacket input{};
        std::memcpy(&input, payload.data(), sizeof(InputPacket));

        auto is_known = (_live_entities.find(input.clientId) != _live_entities.end());
        if (!is_known)
        {
          bool is_player = std::any_of(_players.begin(), _players.end(),
                                       [&](const PlayerInfo &p)
                                       { return p.entityId == input.clientId; });
          if (!is_player)
            continue;
        }
                std::unordered_set<engine::R_Events::Key> keys;
                const size_t expectedSize = sizeof(InputPacket) + static_cast<size_t>(input.keyCount) * sizeof(int32_t);
                if (payload.size() >= expectedSize && input.keyCount > 0)
                {
                    const int32_t *kptr = reinterpret_cast<const int32_t *>(payload.data() + sizeof(InputPacket));
                    for (uint16_t i = 0; i < input.keyCount; ++i)
                        keys.insert(static_cast<engine::R_Events::Key>(kptr[i]));
                }

                for (auto &p : _players)
                {
                    if (p.endpoint == sender && p.entityId == input.clientId)
                    {
                        auto &velocities = _registry.get_components<component::velocity>();
                        if (static_cast<size_t>(p.entityId) < velocities.size() && velocities[p.entityId])
                        {
                            auto &vel = *velocities[p.entityId];
                            using engine::R_Events::Key;
                            bool left = keys.count(Key::Left) > 0 || keys.count(Key::Q) > 0;
                            bool right = keys.count(Key::Right) > 0 || keys.count(Key::D) > 0;
                            bool up = keys.count(Key::Up) > 0 || keys.count(Key::Z) > 0;
                            bool down = keys.count(Key::Down) > 0 || keys.count(Key::S) > 0;
                            vel.vx = left ? -PLAYER_SPEED : right ? PLAYER_SPEED
                                                                  : 0.f;
                            vel.vy = up ? -PLAYER_SPEED : down ? PLAYER_SPEED
                                                               : 0.f;
                        }

                        using engine::R_Events::Key;
                        bool spaceNow = keys.count(Key::Space) > 0;
                        bool cNow = keys.count(Key::C) > 0;
                        bool spacePrev = (_prevSpace.find(p.entityId) != _prevSpace.end()) ? _prevSpace[p.entityId] : false;
                        bool cPrev = (_prevC.find(p.entityId) != _prevC.end()) ? _prevC[p.entityId] : false;
                        constexpr uint32_t CHARGE_TICKS = 30;
                        if (spaceNow && !spacePrev) {
                            _pressTick[p.entityId] = _tick;
                        }
                        if (!spaceNow && spacePrev) {
                            uint32_t start = (_pressTick.find(p.entityId) != _pressTick.end()) ? _pressTick[p.entityId] : _tick;
                            uint32_t held = (_tick > start) ? (_tick - start) : 0;
                            engine::entity_t e = (held >= CHARGE_TICKS) ? spawn_projectile_charged(p.entityId, held) : spawn_projectile_basic(p.entityId);
                            _live_entities.insert(static_cast<uint32_t>(e));
                            _pressTick.erase(p.entityId);
                        }
            if (cNow && !cPrev) {
              auto e = spawn_projectile_bomb(p.entityId);
                            _live_entities.insert(static_cast<uint32_t>(e));
                        }
                        _prevSpace[p.entityId] = spaceNow;
                        _prevC[p.entityId] = cNow;
                        break;
                    }
                }
            }
        }
    }
}

engine::entity_t server::spawn_projectile_basic(engine::entity_t owner)
{
    auto &positions = _registry.get_components<component::position>();
    auto &hitboxes = _registry.get_components<component::hitbox>();
    size_t idx = static_cast<size_t>(owner);
    if (idx >= positions.size() || !positions[idx]) return owner;
    auto pos = positions[idx].value();
    float playerW = 0.f, playerH = 0.f;
    if (idx < hitboxes.size() && hitboxes[idx]) { playerW = hitboxes[idx]->width; playerH = hitboxes[idx]->height; }
    constexpr float w = 10.f, h = 10.f;
    float startX = pos.x + playerW + 4.f;
    float startY = pos.y + (playerH * 0.5f) - (h * 0.5f);
    return engine::make_entity(
        _registry,
    component::position{startX, startY}, component::velocity{1.f, 0.f},
        component::hitbox{w, h},
        component::collision_state{false},
        component::entity_kind::projectile,
        component::projectile_tag{static_cast<uint32_t>(owner), 120, 1.f, 0.f, 2.0f, 2},
        component::health{1});
}

engine::entity_t server::spawn_projectile_alt(engine::entity_t owner)
{
    auto &positions = _registry.get_components<component::position>();
    auto &hitboxes = _registry.get_components<component::hitbox>();
    size_t idx = static_cast<size_t>(owner);
    if (idx >= positions.size() || !positions[idx]) return owner;
    auto pos = positions[idx].value();
    float playerW = 0.f, playerH = 0.f;
    if (idx < hitboxes.size() && hitboxes[idx]) { playerW = hitboxes[idx]->width; playerH = hitboxes[idx]->height; }
    constexpr float w = 12.f, h = 12.f;
    float startX = pos.x + playerW + 4.f;
    float startY = pos.y + (playerH * 0.5f) - (h * 0.5f);
    return engine::make_entity(
        _registry,
    component::position{startX, startY}, component::velocity{1.f, 0.f},
        component::hitbox{w, h},
        component::collision_state{false},
        component::entity_kind::projectile,
        component::projectile_tag{static_cast<uint32_t>(owner), 120, 1.f, 0.f, 3.0f, 2},
        component::health{1});
}

engine::entity_t server::spawn_projectile_charged(engine::entity_t owner, uint32_t heldTicks)
{
    auto &positions = _registry.get_components<component::position>();
    auto &hitboxes = _registry.get_components<component::hitbox>();
    size_t idx = static_cast<size_t>(owner);
    if (idx >= positions.size() || !positions[idx])
        return owner;
    auto pos = positions[idx].value();
    float playerW = 0.f, playerH = 0.f;
    if (idx < hitboxes.size() && hitboxes[idx]) {
        playerW = hitboxes[idx]->width;
        playerH = hitboxes[idx]->height;
    }
    float scale = std::min(1.0f + (heldTicks / 60.0f), 3.0f);
    float w = 14.f * scale, h = 14.f * scale;
    float speed = 5.5f + 1.0f * scale;
    int dmg = static_cast<int>(2 * scale) + 1;
    float startX = pos.x + playerW + 4.f;
    float startY = pos.y + (playerH * 0.5f) - (h * 0.5f);
    return engine::make_entity(
        _registry,
    component::position{startX, startY}, component::velocity{speed, 0.f},
        component::hitbox{w, h},
        component::collision_state{false},
        component::entity_kind::projectile_charged,
        component::projectile_tag{static_cast<uint32_t>(owner), 180, 1.f, 0.f, speed, dmg},
        component::health{1});
}

engine::entity_t server::spawn_projectile_bomb(engine::entity_t owner)
{
  auto &positions = _registry.get_components<component::position>();
  auto &hitboxes = _registry.get_components<component::hitbox>();
  size_t idx = static_cast<size_t>(owner);
  if (idx >= positions.size() || !positions[idx])
    return owner;
  auto pos = positions[idx].value();
  float playerW = 0.f, playerH = 0.f;
  if (idx < hitboxes.size() && hitboxes[idx]) { 
    playerW = hitboxes[idx]->width;
    playerH = hitboxes[idx]->height;
  }

  constexpr float w = 17.f, h = 17.f;
  float startX = pos.x + playerW - w * 0.2f;
  float startY = pos.y + (playerH * 0.5f) - (h * 0.5f);

  float dirX = 0.8f;
  float dirY = -0.9f;
  float speed = 2.2f;
  uint32_t lifetime = 240;
  int damage = 3;

  return engine::make_entity(
    _registry,
    component::position{startX, startY}, component::velocity{dirX * speed, dirY * speed},
    component::hitbox{w, h},
    component::collision_state{false},
    component::entity_kind::projectile_bomb,
    component::projectile_tag{static_cast<uint32_t>(owner), lifetime, dirX, dirY, speed, damage},
    component::gravity{0.03f},
    component::health{1}
  );
}

engine::entity_t server::spawn_player(engine::net::Endpoint endpoint, std::size_t index)
{
  float spawnX = 100.f;
  float spawnY = 100.f + 120.f * static_cast<float>(index);
  auto eid = engine::make_entity(
      _registry, component::position{spawnX, spawnY}, component::velocity{0, 0},
      component::hitbox{34, 20}, component::controllable{},
      component::collision_state{false}, component::health{20},
      component::damage{0}, component::entity_kind::player,
      component::controlled_by{static_cast<uint32_t>(index)},
      component::damage_cooldown{0});
  return eid;
}

engine::entity_t server::spawn_projectile(engine::entity_t owner)
{
    auto &positions = _registry.get_components<component::position>();
    auto &hitboxes = _registry.get_components<component::hitbox>();
    size_t idx = static_cast<size_t>(owner);
    if (idx >= positions.size() || !positions[idx])
        return owner;
    auto pos = positions[idx].value();
    float playerW = 0.f, playerH = 0.f;
    if (idx < hitboxes.size() && hitboxes[idx])
    {
        playerW = hitboxes[idx]->width;
        playerH = hitboxes[idx]->height;
    }
    constexpr float projectileW = 24.f;
    constexpr float projectileH = 20.f;
    float startX = pos.x + playerW + 4.f;
    float startY = pos.y + (playerH * 0.5f) - (projectileH * 0.5f);
    auto proj = engine::make_entity(
        _registry,
    component::position{startX, startY}, component::velocity{2.f, 0.f},
        component::hitbox{projectileW, projectileH},
        component::collision_state{false},
        component::entity_kind::projectile,
        component::projectile_tag{static_cast<uint32_t>(owner), 120, 1.f, 0.f, 2.f, 2},
        component::health{1});
    return proj;
}

engine::entity_t server::spawn_missile_explosion(float x, float y, int damage, float radius)
{
  float size = radius * 2.f;
  float topLeftX = x - radius;
  float topLeftY = y - radius;
  auto e = engine::make_entity(
    _registry,
    component::position{topLeftX, topLeftY},
    component::velocity{0.f, 0.f},
    component::hitbox{size, size},
    component::collision_state{false},
    component::entity_kind::missile_explosion,
    component::projectile_tag{0u, 30u, 0.f, 0.f, 0.f, damage}, // ~0.5s lifetime to match animation
    component::area_effect{radius, damage, false},
    component::health{1});
  return e;
}
