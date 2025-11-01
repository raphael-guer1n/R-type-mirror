#include "server/Server.hpp"
#include "server/Components_ai.hpp"
#include "server/System_ai.hpp"
#include "common/Systems.hpp"
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
