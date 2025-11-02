/**
 * @brief Registers the Bounds System within the ECS registry.
 *
 * This system ensures all entities remain within the valid game world area
 * and removes those that travel far beyond the visible screen boundaries.
 * It handles both **projectiles** and **actors** (players/enemies) differently.
 *
 * ### Behavior:
 * - **Projectiles (player/enemy)**:
 *   - Destroyed if they go too far off-screen (±50 units beyond window bounds).
 *   - Removed from `_live_entities` to keep entity tracking consistent.
 * - **Enemies**:
 *   - If they move beyond -90 on the X-axis, they are repositioned to the
 *     right side of the screen (`SCREEN_WIDTH + 100`) — useful for looping spawns.
 * - **Players and others**:
 *   - Their position is clamped within the visible screen (0 ≤ x ≤ SCREEN_WIDTH, 0 ≤ y ≤ SCREEN_HEIGHT).
 *   - Velocity is reset to `0` if an entity hits a boundary to prevent continuous movement out of bounds.
 *
 * ### Components Used:
 * - **component::position** — Entity’s current coordinates.
 * - **component::velocity** — Movement vector, reset when hitting edges.
 * - **component::entity_kind** — Used to differentiate projectiles, enemies, and other entity types.
 *
 * ### Constants Used:
 * - `SCREEN_WIDTH` — Game world width.
 * - `SCREEN_HEIGHT` — Game world height.
 *
 * @note This system should execute after all movement systems so that corrections
 *       and cleanups are applied at the end of each frame.
 *
 * @warning Deletion of entities modifies `_live_entities`; ensure this
 *          container stays synchronized with game state and network updates.
 */
#include "server/Server.hpp"
#include "server/Components_ai.hpp"
#include "server/System_ai.hpp"
#include "engine/ecs/Systems.hpp"
#include "server/ServerUtils.hpp"
#include "server/GameLogic.hpp"

void GameLogic::register_bounds_system()
{
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

          if (kind == component::entity_kind::playerProjectile || kind == component::entity_kind::enemyProjectile)
          {
            if (x < -50.f || x > SCREEN_WIDTH + 50.f || y < -50.f || y > SCREEN_HEIGHT + 50.f)
            {
              toKill.push_back(reg.entity_from_index(i));
            }
            continue;
          }

          bool corrected = false;
          if (x < -90.f && kind == component::entity_kind::enemy) {
            x = SCREEN_WIDTH + 100;
            corrected = true;
          }
          else if (x < 0.f && kind != component::entity_kind::enemy) { x = 0.f; corrected = true; }
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
