#include "server/Server.hpp"
#include "server/Components_ai.hpp"
#include "server/System_ai.hpp"
#include "common/Systems.hpp"
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
