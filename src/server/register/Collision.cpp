#include "server/Server.hpp"
#include "server/Components_ai.hpp"
#include "server/System_ai.hpp"
#include "common/Systems.hpp"
#include "server/ServerUtils.hpp"
#include "server/SpawnProjectile.hpp"
#include "server/GameLogic.hpp"

void GameLogic::register_collision_system()
{
    _registry.add_system<component::position, component::hitbox>(
        [this](engine::registry &reg,
            engine::sparse_array<component::position> &positions,
            engine::sparse_array<component::hitbox> &hitboxes)
        {
            auto &collisions = _registry.get_components<component::collision_state>();
            auto &velocities = _registry.get_components<component::velocity>();
            auto &kinds = _registry.get_components<component::entity_kind>();
            auto &damages = _registry.get_components<component::damage>();
            auto &cooldowns = _registry.get_components<component::damage_cooldown>();
            auto &projectiles = _registry.get_components<component::projectile_tag>();
            std::vector<bool> newCollided(collisions.size(), false);

            hitbox_system(reg, positions, hitboxes, [&](std::size_t i,
                std::size_t j) {
                auto kindI = (i < kinds.size() && kinds[i]) ? kinds[i].value() : component::entity_kind::unknown;
                auto kindJ = (j < kinds.size() && kinds[j]) ? kinds[j].value() : component::entity_kind::unknown;

                if (kindI == component::entity_kind::player && kindJ == component::entity_kind::enemy) {
                    serverutils::apply_damage_with_cooldown(i, _tick, reg, damages, cooldowns, collisions);
                    newCollided[i] = true;
                    serverutils::resolve_block(i, j, positions, hitboxes, collisions, velocities);
                }
                if (kindJ == component::entity_kind::player && kindI == component::entity_kind::enemy) {
                    serverutils::apply_damage_with_cooldown(j, _tick, reg, damages, cooldowns, collisions);
                    newCollided[j] = true;
                    serverutils::resolve_block(j, i, positions, hitboxes, collisions, velocities);
                }
                if ((kindI == component::entity_kind::playerProjectile || kindI == component::entity_kind::projectile_bomb || kindI == component::entity_kind::projectile_charged) &&
                    kindJ == component::entity_kind::enemy) {
                    if (i < projectiles.size() && projectiles[i]) {
                        auto &proj = projectiles[i].value();
                        auto ownerKind = (proj.owner < kinds.size() && kinds[proj.owner]) ? kinds[proj.owner].value() : component::entity_kind::unknown;
                        if (ownerKind != component::entity_kind::enemy) {
                            if (j < damages.size() && damages[j]) damages[j]->amount += proj.damage;
                            else reg.add_component(reg.entity_from_index(j), component::damage{proj.damage});
                                if (kindI == component::entity_kind::projectile_bomb) {
                                auto &posArr = _registry.get_components<component::position>();
                                if (i < posArr.size() && posArr[i]) {
                                    auto pPos = posArr[i].value();
                                    auto exp = spawn_missile_explosion(pPos.x, pPos.y, proj.damage, 180.f, reg);
                                    _live_entities.insert(static_cast<uint32_t>(exp));
                                }
                            }
                            _live_entities.erase(static_cast<uint32_t>(reg.entity_from_index(i)));
                            reg.kill_entity(reg.entity_from_index(i));
                        }
                    }
                }

                if ((kindJ == component::entity_kind::playerProjectile || kindJ == component::entity_kind::projectile_charged || kindJ == component::entity_kind::projectile_bomb) &&
                    kindI == component::entity_kind::enemy)
                {
                    if (j < projectiles.size() && projectiles[j])
                    {
                    auto &proj = projectiles[j].value();
                    auto ownerKind = (proj.owner < kinds.size() && kinds[proj.owner]) ? kinds[proj.owner].value() : component::entity_kind::unknown;
                    if (ownerKind != component::entity_kind::enemy)
                    {
                        if (i < damages.size() && damages[i]) damages[i]->amount += proj.damage;
                        else reg.add_component(reg.entity_from_index(i), component::damage{proj.damage});
                        if (kindJ == component::entity_kind::projectile_bomb)
                        {
                        auto &posArr = _registry.get_components<component::position>();
                        if (j < posArr.size() && posArr[j])
                        {
                            auto pPos = posArr[j].value();
                            auto exp = spawn_missile_explosion(pPos.x, pPos.y, proj.damage, 180.f, reg);
                            _live_entities.insert(static_cast<uint32_t>(exp));
                        }
                        }
                        _live_entities.erase(static_cast<uint32_t>(reg.entity_from_index(j)));
                        reg.kill_entity(reg.entity_from_index(j));
                    }
                    }
                }

                if ((kindI == component::entity_kind::enemyProjectile || kindI == component::entity_kind::projectile_bomb) && kindJ == component::entity_kind::player)
                {
                    if (i < projectiles.size() && projectiles[i])
                    {
                    auto &proj = projectiles[i].value();
                    auto ownerKind = (proj.owner < kinds.size() && kinds[proj.owner]) ? kinds[proj.owner].value() : component::entity_kind::unknown;
                    if (ownerKind != component::entity_kind::player)
                    {
                        if (j < damages.size() && damages[j]) damages[j]->amount += proj.damage;
                        else reg.add_component(reg.entity_from_index(j), component::damage{proj.damage});
                        _live_entities.erase(static_cast<uint32_t>(reg.entity_from_index(i)));
                        reg.kill_entity(reg.entity_from_index(i));
                    }
                    }
                }

                if ((kindJ == component::entity_kind::enemyProjectile || kindJ == component::entity_kind::projectile_bomb) && kindI == component::entity_kind::player)
                {
                    if (j < projectiles.size() && projectiles[j])
                    {
                    auto &proj = projectiles[j].value();
                    auto ownerKind = (proj.owner < kinds.size() && kinds[proj.owner]) ? kinds[proj.owner].value() : component::entity_kind::unknown;
                    if (ownerKind != component::entity_kind::player)
                    {
                        if (i < damages.size() && damages[i]) damages[i]->amount += proj.damage;
                        else reg.add_component(reg.entity_from_index(i), component::damage{proj.damage});
                        _live_entities.erase(static_cast<uint32_t>(reg.entity_from_index(j)));
                        reg.kill_entity(reg.entity_from_index(j));
                    }
                    }
                }
        });

            for (std::size_t idx = 0; idx < collisions.size(); ++idx)
            {
                if (collisions[idx])
                    collisions[idx]->collided = newCollided[idx];
            }
        });
}
