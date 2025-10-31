#include "server/Server.hpp"
#include "server/System_ai.hpp"
#include "common/Systems.hpp"

void server::register_projectile_movement_system()
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