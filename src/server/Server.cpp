
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
#include "Server.hpp"
#include "server/Server.hpp"
#include "common/Systems.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "server/ServerUtils.hpp"
#include "enemies/Crawler.hpp"
#include "enemies/Shooter.hpp"
#include "enemies/Boss.hpp"

using json = nlohmann::json;

using namespace serverutils;

server::server(unsigned short port)
: _netServer(port)
{
  _netServer.set_packet_handler([this](const engine::net::Endpoint& sender,
  const PacketHeader& hdr, const std::vector<uint8_t>& payLoad) {
    this->on_packet_received(sender, hdr, payLoad);
  });
  register_components();
}

void server::run()
{
  _netServer.start();
  std::cout << "Waiting for players..." << std::endl;

  _running = true;
  using clock = std::chrono::steady_clock;
  const auto tick_duration = std::chrono::milliseconds(16);
  auto last_tick = clock::now();

  while (_running)
  {
    _netServer.poll();
    if (_ready) {
      auto now = clock::now();
      if (now - last_tick >= tick_duration)
      {
        update_game_logic();
        update_spawns_and_events();
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
  _netServer.stop();
}

void server::update_spawns_and_events()
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
    if (_tick % 600 == 0)
    {
        Enemies::Crawler::NewCrawler(_registry, _live_entities, _gen);
        Enemies::Shooter::NewShooter(_registry, _live_entities, _gen);
        Enemies::Boss::NewBoss(_registry, _live_entities, _gen);
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
    SnapshotBuilderContext ctx{positions, velocities, kinds,
        collisions, healths, hitboxes};
    for (auto &pInfo : _players) {
        try_add_entity(static_cast<uint32_t>(pInfo.entityId), states,
            ctx, inserted, SNAPSHOT_LIMIT);
    }
    for (uint32_t entityId : _live_entities) {
        if (states.size() >= SNAPSHOT_LIMIT)
            break;
        try_add_entity(entityId, states, ctx, inserted, SNAPSHOT_LIMIT);
    }
    if (states.empty()) return;

    Snapshot snap{_tick, static_cast<uint16_t>(states.size())};
    std::vector<uint8_t> buf(sizeof(Snapshot) + sizeof(EntityState)
        * states.size());
    std::memcpy(buf.data(), &snap, sizeof(Snapshot));
    std::memcpy(buf.data() + sizeof(Snapshot), states.data(),
        sizeof(EntityState) * states.size());
    PacketHeader hdr{SNAPSHOT, static_cast<uint16_t>(buf.size()), _tick};

    for (auto &p : _players)
        _netServer.send(hdr, buf, p.endpoint);
}

void server::broadcast_game_over(uint32_t winnerEntityId)
{
    GameOverPayload payload{winnerEntityId};
    PacketHeader hdr{
        GAME_OVER,
        static_cast<uint16_t>(sizeof(payload)),
        _tick};
    std::vector<uint8_t> data(sizeof(payload));
    std::memcpy(data.data(), &payload, sizeof(payload));
    for (auto &p : _players)
        _netServer.send(hdr, data, p.endpoint);
    std::cout << "Game Over! Winner entity id: " << winnerEntityId << std::endl;
}

void server::check_game_over()
{
  auto &healths = _registry.get_components<component::health>();
  auto &kinds = _registry.get_components<component::entity_kind>();

  std::vector<uint32_t> alivePlayers;

  for (auto &&[i, kind] : indexed_zipper(kinds))
  {
    if (kind != component::entity_kind::player)
      continue;
    if (i < healths.size() && healths[i] && healths[i]->hp > 0)
    {
      alivePlayers.push_back(static_cast<uint32_t>(i));
    }
  }
  if (alivePlayers.size() <= 1)
  {
    uint32_t winnerId = alivePlayers.empty() ? UINT32_MAX : alivePlayers[0];
    broadcast_game_over(winnerId);
    _running = false;
  }
}

void server::update_game_logic()
{
  constexpr float delta = 1.f / 60.f;
  auto &positions = _registry.get_components<component::position>();
  auto &velocities = _registry.get_components<component::velocity>();

  position_system(_registry, positions, velocities, delta);
  _registry.run_systems();
  check_game_over();
}

engine::entity_t server::spawn_player(engine::net::Endpoint endpoint, std::size_t index)
{
  float spawnX = 100.f;
  float spawnY = 100.f + 120.f * static_cast<float>(index);
  auto eid = engine::make_entity(
      _registry, component::position{spawnX, spawnY}, component::velocity{0, 0},
      component::hitbox{124, 70}, component::controllable{},
      component::collision_state{false}, component::health{20},
      component::damage{0}, component::entity_kind::player,
      component::controlled_by{static_cast<uint32_t>(index)},
      component::damage_cooldown{0});
  return eid;
}

void server::on_packet_received(const engine::net::Endpoint &sender,
  const PacketHeader &hdr, const std::vector<uint8_t> &payload)
{
  switch (hdr.type) {
    case CONNECT_REQ: handle_connect(sender); break;
    case INPUT_PKT: handle_input(sender, payload); break;
    default:
      break;
  }
}

void server::setup_systems()
{
  register_health_and_spawn_systems();
  register_projectile_movement_system();
  register_gravity_system();
  register_collision_system();
  register_bounds_system();
  register_area_effect_system();
}

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
