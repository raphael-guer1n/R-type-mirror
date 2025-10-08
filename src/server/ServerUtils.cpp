#include "server/ServerUtils.hpp"
#include "common/Packets.hpp"
#include <algorithm>

namespace serverutils {

void resolve_block(std::size_t moverIdx, std::size_t blockerIdx,
                   engine::sparse_array<component::position> &positions,
                   engine::sparse_array<component::hitbox> &hitboxes,
                   engine::sparse_array<component::collision_state> &collisions,
                   engine::sparse_array<component::velocity> &velocities) {
  if (moverIdx >= collisions.size() || !collisions[moverIdx] ||
      !positions[moverIdx] || !hitboxes[moverIdx] ||
      blockerIdx >= positions.size() || !positions[blockerIdx] ||
      !hitboxes[blockerIdx])
    return;

  collisions[moverIdx]->collided = true;
  auto &moverPos = positions[moverIdx].value();
  const auto &moverHitbox = hitboxes[moverIdx].value();
  const auto &blockPos = positions[blockerIdx].value();
  const auto &blockHitbox = hitboxes[blockerIdx].value();

  // Compute corners using center-based coordinates
  float ax1 = moverPos.x - moverHitbox.width * 0.5f + moverHitbox.offset_x;
  float ay1 = moverPos.y - moverHitbox.height * 0.5f + moverHitbox.offset_y;
  float ax2 = ax1 + moverHitbox.width;
  float ay2 = ay1 + moverHitbox.height;
  float bx1 = blockPos.x - blockHitbox.width * 0.5f + blockHitbox.offset_x;
  float by1 = blockPos.y - blockHitbox.height * 0.5f + blockHitbox.offset_y;
  float bx2 = bx1 + blockHitbox.width;
  float by2 = by1 + blockHitbox.height;

  float overlapX = std::min(ax2, bx2) - std::max(ax1, bx1);
  float overlapY = std::min(ay2, by2) - std::max(ay1, by1);

  if (overlapX <= 0.f || overlapY <= 0.f)
    return;

  float acx = moverPos.x;
  float bcx = blockPos.x;
  float acy = moverPos.y;
  float bcy = blockPos.y;

  if (overlapX < overlapY) {
    if (acx < bcx)
      moverPos.x -= overlapX;
    else
      moverPos.x += overlapX;
    if (moverIdx < velocities.size() && velocities[moverIdx])
      velocities[moverIdx]->vx = 0.f;
  } else {
    if (acy < bcy)
      moverPos.y -= overlapY;
    else
      moverPos.y += overlapY;
    if (moverIdx < velocities.size() && velocities[moverIdx])
      velocities[moverIdx]->vy = 0.f;
  }
}

void apply_damage_with_cooldown(
    std::size_t entityIndex, uint32_t currentTick, engine::registry &reg,
    engine::sparse_array<component::damage> &damages,
    engine::sparse_array<component::damage_cooldown> &cooldowns,
    engine::sparse_array<component::collision_state> &collisions) {
  if (entityIndex >= damages.size())
    return;

  uint32_t lastHitTick =
      (entityIndex < cooldowns.size() && cooldowns[entityIndex])
          ? cooldowns[entityIndex]->last_hit_tick
          : 0;
  if (currentTick <= lastHitTick + 60)
    return;

  if (!damages[entityIndex])
    reg.add_component(reg.entity_from_index(entityIndex), component::damage{1});
  else
    damages[entityIndex]->amount += 1;

  if (entityIndex < cooldowns.size() && cooldowns[entityIndex])
    cooldowns[entityIndex]->last_hit_tick = currentTick;
  else
    reg.add_component(reg.entity_from_index(entityIndex),
                      component::damage_cooldown{currentTick});

  if (entityIndex < collisions.size() && collisions[entityIndex])
    collisions[entityIndex]->collided = true;
}

void try_add_entity(uint32_t entityId, std::vector<EntityState> &out,
                    SnapshotBuilderContext &ctx,
                    std::unordered_set<uint32_t> &inserted, std::size_t limit) {
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

  if (idx < ctx.velocities.size() && ctx.velocities[idx]) {
    es.vx = ctx.velocities[idx]->vx;
    es.vy = ctx.velocities[idx]->vy;
  } else {
    es.vx = 0.f;
    es.vy = 0.f;
  }

  if (idx < ctx.kinds.size() && ctx.kinds[idx])
    es.type = static_cast<uint8_t>(ctx.kinds[idx].value());
  else
    es.type = static_cast<uint8_t>(component::entity_kind::unknown);

  if (idx < ctx.healths.size() && ctx.healths[idx])
    es.hp = ctx.healths[idx]->hp;
  else
    es.hp = 100;

  es.collided = (idx < ctx.collisions.size() && ctx.collisions[idx] &&
                 ctx.collisions[idx]->collided)
                    ? 1
                    : 0;

  out.push_back(es);
  inserted.insert(entityId);
}

} // namespace serverutils