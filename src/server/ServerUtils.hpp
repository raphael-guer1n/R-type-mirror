#pragma once
#include "engine/ecs/Registry.hpp"
#include "common/Components.hpp"
#include "common/Packets.hpp" // for EntityState
#include <vector>
#include <unordered_set>
#include <cstdint>

namespace serverutils {

void resolve_block(
    std::size_t moverIdx,
    std::size_t blockerIdx,
    engine::sparse_array<component::position> &positions,
    engine::sparse_array<component::hitbox> &hitboxes,
    engine::sparse_array<component::collision_state> &collisions,
    engine::sparse_array<component::velocity> &velocities);

void apply_damage_with_cooldown(
    std::size_t entityIndex,
    uint32_t currentTick,
    engine::registry &reg,
    engine::sparse_array<component::damage> &damages,
    engine::sparse_array<component::damage_cooldown> &cooldowns,
    engine::sparse_array<component::collision_state> &collisions);

struct SnapshotBuilderContext {
    engine::sparse_array<component::position> &positions;
    engine::sparse_array<component::entity_kind> &kinds;
    engine::sparse_array<component::collision_state> &collisions;
    engine::sparse_array<component::health> &healths;
};

void try_add_entity(
    uint32_t entityId,
    std::vector<EntityState> &out,
    SnapshotBuilderContext &ctx,
    std::unordered_set<uint32_t> &inserted,
    std::size_t limit);

} // namespace serverutils
