#pragma once
#include "engine/ecs/Registry.hpp"
#include "common/Components.hpp"
#include "common/Packets.hpp" // for EntityState
#include <vector>
#include <unordered_set>
#include <cstdint>

 // namespace serverutils
/**
 * @file ServerUtils.hpp
 * @brief Utility functions and structures for server-side entity management in the R-Type game.
 *
 * This header provides helper functions for resolving entity collisions, applying damage with cooldowns,
 * and building entity snapshots for network synchronization. It also defines a context structure for
 * snapshot building.
 */

namespace serverutils {

/**
 * @brief Resolves a blocking collision between two entities.
 *
 * Adjusts positions, velocities, and collision states of the mover and blocker entities
 * based on their hitboxes and current states.
 *
 * @param moverIdx Index of the moving entity.
 * @param blockerIdx Index of the blocking entity.
 * @param positions Reference to the array of position components.
 * @param hitboxes Reference to the array of hitbox components.
 * @param collisions Reference to the array of collision state components.
 * @param velocities Reference to the array of velocity components.
 */
void resolve_block(
    std::size_t moverIdx,
    std::size_t blockerIdx,
    engine::sparse_array<component::position> &positions,
    engine::sparse_array<component::hitbox> &hitboxes,
    engine::sparse_array<component::collision_state> &collisions,
    engine::sparse_array<component::velocity> &velocities);

/**
 * @brief Applies damage to an entity if its cooldown has expired.
 *
 * Checks the damage cooldown for the entity and applies damage if allowed.
 * Updates the cooldown and collision state accordingly.
 *
 * @param entityIndex Index of the entity to apply damage to.
 * @param currentTick Current server tick for cooldown timing.
 * @param reg Reference to the ECS registry.
 * @param damages Reference to the array of damage components.
 * @param cooldowns Reference to the array of damage cooldown components.
 * @param collisions Reference to the array of collision state components.
 */
void apply_damage_with_cooldown(
    std::size_t entityIndex,
    uint32_t currentTick,
    engine::registry &reg,
    engine::sparse_array<component::damage> &damages,
    engine::sparse_array<component::damage_cooldown> &cooldowns,
    engine::sparse_array<component::collision_state> &collisions);
/**
 * @brief Context structure for building entity snapshots.
 *
 * Holds references to relevant component arrays needed for snapshot construction.
 */
struct SnapshotBuilderContext {
    engine::sparse_array<component::position> &positions;
    engine::sparse_array<component::entity_kind> &kinds;
    engine::sparse_array<component::collision_state> &collisions;
    engine::sparse_array<component::health> &healths;
};
/**
 * @brief Attempts to add an entity to the snapshot output.
 *
 * Adds the entity's state to the output vector if it hasn't already been inserted and
 * the limit has not been reached.
 *
 * @param entityId ID of the entity to add.
 * @param out Output vector of entity states.
 * @param ctx Context containing component arrays.
 * @param inserted Set of already inserted entity IDs.
 * @param limit Maximum number of entities to add.
 */
void try_add_entity(
    uint32_t entityId,
    std::vector<EntityState> &out,
    SnapshotBuilderContext &ctx,
    std::unordered_set<uint32_t> &inserted,
    std::size_t limit);

}