#include "server/Server.hpp"
#include "server/Components_ai.hpp"
#include "server/System_ai.hpp"
#include "common/Systems.hpp"
#include "server/ServerUtils.hpp"

void server::register_area_effect_system()
{
  _registry.add_system<component::position, component::area_effect, component::entity_kind>(
      [this](engine::registry &reg,
             engine::sparse_array<component::position> &positions,
             engine::sparse_array<component::area_effect> &areas,
             engine::sparse_array<component::entity_kind> &kinds) {
        auto &damages = _registry.get_components<component::damage>();
        for (auto &&[i, pos, area, kind] : indexed_zipper(positions, areas, kinds))
        {
          (void)i;
          if (kind != component::entity_kind::missile_explosion) continue;
          if (area.applied) continue;
          auto &kindsArr = _registry.get_components<component::entity_kind>();
          auto &posArr = _registry.get_components<component::position>();
          for (size_t j = 0; j < kindsArr.size(); ++j)
          {
            if (j >= posArr.size() || !kindsArr[j] || !posArr[j]) continue;
            if (kindsArr[j].value() != component::entity_kind::enemy) continue;
            float centerX = pos.x + (area.radius);
            float centerY = pos.y + (area.radius);
            auto ep = posArr[j].value();
            float dx = ep.x - centerX;
            float dy = ep.y - centerY;
            if ((dx * dx + dy * dy) <= area.radius * area.radius)
            {
              if (j < damages.size() && damages[j]) damages[j]->amount += area.damage;
              else reg.add_component(reg.entity_from_index(j), component::damage{area.damage});
            }
          }
          area.applied = true;
        }
      });
}