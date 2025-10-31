#include "Server.hpp"
#include "SpawnProjectile.hpp"
#include "engine/events/Events.hpp"

void server::handle_input(const engine::net::Endpoint &sender,
  const std::vector<uint8_t> &payload)
{
  if (payload.size() < sizeof(InputPacket))
    return;

  InputPacket input{};
  std::memcpy(&input, payload.data(), sizeof(InputPacket));

  auto it = std::find_if(_players.begin(), _players.end(),
    [&](const PlayerInfo &p)
    { return p.endpoint == sender; });

  if (it == _players.end())
    return;

  const auto &player = *it;
  auto entityId = player.entityId;

  // Decode pressed keys
  std::unordered_set<engine::R_Events::Key> keys;
  const size_t expectedSize = sizeof(InputPacket) +
    static_cast<size_t>(input.keyCount) * sizeof(int32_t);
  if (payload.size() >= expectedSize && input.keyCount > 0)
  {
    const int32_t *kptr = reinterpret_cast<const int32_t *>(
      payload.data() + sizeof(InputPacket));
    for (uint16_t i = 0; i < input.keyCount; ++i)
      keys.insert(static_cast<engine::R_Events::Key>(kptr[i]));
  }

  // --- Movement logic ---
  auto &velocities = _registry.get_components<component::velocity>();
  if (static_cast<size_t>(entityId) < velocities.size() && velocities[entityId])
  {
    auto &vel = *velocities[entityId];
    using engine::R_Events::Key;

    bool left = keys.count(Key::Left) || keys.count(Key::Q);
    bool right = keys.count(Key::Right) || keys.count(Key::D);
    bool up = keys.count(Key::Up) || keys.count(Key::Z);
    bool down = keys.count(Key::Down) || keys.count(Key::S);
    vel.vx = left ? -PLAYER_SPEED : right ? PLAYER_SPEED: 0.f;
    vel.vy = up ? -PLAYER_SPEED : down ? PLAYER_SPEED: 0.f;
  }

  // --- Shooting / ability logic ---
  using engine::R_Events::Key;
  bool spaceNow = keys.count(Key::Space) > 0;
  bool cNow = keys.count(Key::C) > 0;
  bool spacePrev = (_prevSpace.find(entityId) != _prevSpace.end()) ? _prevSpace[entityId] : false;
  bool cPrev = (_prevC.find(entityId) != _prevC.end()) ? _prevC[entityId] : false;
  constexpr uint32_t CHARGE_TICKS = 30;
  if (spaceNow && !spacePrev)
  {
    _pressTick[entityId] = _tick;
  }
  if (!spaceNow && spacePrev)
  {
    uint32_t start = (_pressTick.find(entityId) != _pressTick.end()) ? _pressTick[entityId] : _tick;
    uint32_t held = (_tick > start) ? (_tick - start) : 0;
    engine::entity_t proj = (held >= CHARGE_TICKS)
      ? spawn_projectile_charged(entityId, held, _registry)
      : spawn_projectile_basic(entityId, _registry);
    _live_entities.insert(static_cast<uint32_t>(proj));
    _pressTick.erase(entityId);
  }
  if (cNow && !cPrev)
  {
    auto bomb = spawn_projectile_bomb(entityId, _registry);
    _live_entities.insert(static_cast<uint32_t>(bomb));
  }
  _prevSpace[entityId] = spaceNow;
  _prevC[entityId] = cNow;
}