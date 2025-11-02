/**
 * @brief Registers the Gravity System for projectiles affected by gravity.
 *
 * This system updates the velocity of all entities that have both a
 * `component::projectile_tag` and a `component::gravity` component.
 * It simulates a simple gravity effect by modifying the projectile’s
 * vertical direction (`dirY`) over time, which in turn changes its
 * velocity components (`vx`, `vy`).
 *
 * ### Overview
 * - The system applies gravitational acceleration (`g.ay`) to each
 *   projectile’s vertical direction (`dirY`).
 * - It then recalculates the projectile’s velocity based on its
 *   direction vector and speed.
 *
 * ### Mathematical Model
 * ```
 * dirY = dirY + g.ay
 * vx = dirX * speed
 * vy = dirY * speed
 * ```
 * - `dirX`, `dirY`: Directional unit vector of the projectile.
 * - `g.ay`: Vertical acceleration due to gravity (usually a small negative value).
 * - `speed`: The projectile’s base movement speed.
 *
 * ### Components Accessed
 * | Component | Purpose |
 * |------------|----------|
 * | `component::projectile_tag` | Contains direction (`dirX`, `dirY`), speed, and owner data. |
 * | `component::gravity` | Holds acceleration data (`ay`). |
 * | `component::velocity` | Updated based on projectile direction and speed. |
 *
 * ### Behavior
 * - Only projectiles tagged with `component::gravity` are affected.
 * - Non-gravitational projectiles (e.g., lasers or straight shots) are excluded.
 * - Direction (`dirY`) continuously changes, allowing for realistic parabolic motion.
 * - The velocity components are recomputed every tick to reflect the updated trajectory.
 *
 * ### Notes
 * - Should be executed **before** movement systems to ensure positions are updated
 *   using the latest velocity values.
 * - Assumes gravity acts **downwards** (positive `ay` increases downward velocity).
 * - The system is simple but can be extended to support custom gravity per entity
 *   (e.g., black holes, upward thrust zones, etc.).
 *
 * ### Example
 * ```
 * Projectile initially: dirY = -0.5, speed = 600, ay = 0.03
 * After one tick:
 *     dirY = -0.47
 *     vx = dirX * 600
 *     vy = -0.47 * 600
 * ```
 */
#include "server/Server.hpp"
#include "server/Components_ai.hpp"
#include "server/System_ai.hpp"
#include "engine/ecs/Systems.hpp"
#include "server/GameLogic.hpp"

void GameLogic::register_gravity_system()
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
