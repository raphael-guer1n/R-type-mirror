/**
 * @brief Registers the Projectile Movement system into the ECS registry.
 *
 * This system is responsible for updating the positions of all projectiles
 * based on their direction and speed, as well as handling their lifetimes.
 * When a projectile's lifetime reaches zero, it is removed from the ECS registry
 * and the server's live entity tracking.
 *
 * ---
 *
 * ### Behavior
 * - Iterates over all entities with both `component::position` and `component::projectile_tag`.
 * - Updates positions according to:
 *   ```
 *   pos.x += proj.dirX * proj.speed
 *   pos.y += proj.dirY * proj.speed
 *   ```
 * - Decrements `proj.lifetime` each tick.
 * - Marks projectiles for removal when `lifetime <= 0`.
 * - Cleans up removed projectiles from `_live_entities` and calls `kill_entity()`.
 *
 * ---
 *
 * ### Components Used
 * | Component | Purpose |
 * |-----------|---------|
 * | `component::position` | Tracks the current position of the projectile. |
 * | `component::projectile_tag` | Stores movement direction, speed, lifetime, and other projectile-specific metadata. |
 *
 * ---
 *
 * ### Notes
 * - This system runs every game tick and is typically executed after input handling
 *   but before collision or bounds systems, so positions are up-to-date for collisions.
 * - Keeping projectile logic modular ensures easy reuse for different projectile types
 *   (e.g., basic, charged, bombs, enemy projectiles).
 *
 * @see engine::registry::add_system
 * @see component::position
 * @see component::projectile_tag
 */

#include "server/Server.hpp"
#include "server/System_ai.hpp"
#include "engine/ecs/Systems.hpp"
#include "server/GameLogic.hpp"

void GameLogic::register_projectile_movement_system()
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