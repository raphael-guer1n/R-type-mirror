/**
 * @brief Registers the core Health and Spawn systems into the ECS registry.
 *
 * This function connects two essential gameplay systems:
 * - **Health System** (`health_system`)
 * - **Spawn System** (`spawn_system`)
 *
 * These systems are responsible for maintaining entity life state and dynamically
 * creating new entities during gameplay, ensuring smooth game flow and event handling.
 *
 * ### 1. Health System
 * Handles entity health and damage application. It typically:
 * - Reduces entity health when `component::damage` is present.
 * - Removes or flags entities for destruction when health reaches zero.
 * - Optionally triggers effects such as explosions, score updates, or game-over events.
 *
 * #### Components Used:
 * | Component | Purpose |
 * |------------|----------|
 * | `component::health` | Tracks an entity's hit points or life value. |
 * | `component::damage` | Represents incoming damage applied to the entity. |
 *
 * #### Example Flow:
 * ```
 * entity.health = 100
 * entity.damage = 30
 * → health_system → entity.health = 70
 * ```
 * When health ≤ 0, the system usually calls `kill_entity()` or a custom destruction handler.
 *
 * ---
 *
 * ### 2. Spawn System
 * Handles entity creation requests at runtime. It processes all entities with
 * a `component::spawn_request` and converts them into active entities in the world.
 *
 * #### Components Used:
 * | Component | Purpose |
 * |------------|----------|
 * | `component::spawn_request` | Contains prefab/type data for the entity to be spawned. |
 *
 * #### Example Flow:
 * ```
 * entity has spawn_request{type = enemy_basic, position = (300, 200)}
 * → spawn_system → spawns enemy entity into registry
 * ```
 *
 * ---
 *
 * ### Execution Order
 * These systems are typically executed early in the game tick:
 * 1. **Health System** – cleans up destroyed entities.
 * 2. **Spawn System** – introduces new entities.
 *
 * This ensures dead entities are removed before new ones spawn in the same tick.
 *
 * ### Notes
 * - These systems are **stateless** and operate directly on ECS components.
 * - The actual logic for `health_system` and `spawn_system` is implemented elsewhere
 *   (usually in `engine/ecs/Systems.hpp` or similar).
 * - Keeping these systems modular allows reuse across multiple game modes or scenes.
 *
 * @see engine::registry::add_system
 * @see component::health
 * @see component::damage
 * @see component::spawn_request
 */
#include "server/Server.hpp"
#include "server/Components_ai.hpp"
#include "server/System_ai.hpp"
#include "engine/ecs/Systems.hpp"
#include "server/GameLogic.hpp"

void GameLogic::register_health_and_spawn_systems()
{
  _registry.add_system<component::health, component::damage>(health_system);
  _registry.add_system<component::spawn_request>(spawn_system);
}
