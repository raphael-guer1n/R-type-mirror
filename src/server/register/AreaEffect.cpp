/**
 * @brief Registers the Area Effect system within the ECS registry.
 *
 * This system handles **area-of-effect damage logic**, typically applied by
 * projectiles or explosions (e.g., missiles). When an entity with both
 * `component::area_effect` and `component::entity_kind` (set to `missile_explosion`)
 * is found, it applies damage to all nearby enemies within the specified radius.
 *
 * ### Process:
 * 1. Iterates through all entities with `position`, `area_effect`, and `entity_kind`.
 * 2. Checks if the entity is a missile explosion that has not yet applied its effect.
 * 3. Calculates distance to all other entities with kind `enemy`.
 * 4. If within the AoE radius, applies or increments a `damage` component on the target.
 * 5. Marks the area effect as applied to avoid duplicate processing.
 *
 * ### Components Used:
 * - **component::position** — World position of both AoE source and targets.
 * - **component::area_effect** — Contains radius and damage amount of the AoE.
 * - **component::entity_kind** — Used to identify the type of entity (e.g., missile, enemy).
 * - **component::damage** — Created or incremented on affected entities.
 *
 * @note This system runs server-side only and should be executed once per tick
 * after collision and projectile updates.
 *
 * @warning Modifying component arrays during iteration (other than `damage`) should
 * be avoided to prevent invalid references.
 */
#include "server/Server.hpp"
#include "server/Components_ai.hpp"
#include "server/System_ai.hpp"
#include "engine/ecs/Systems.hpp"
#include "server/ServerUtils.hpp"
#include "server/GameLogic.hpp"

void GameLogic::register_area_effect_system()
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