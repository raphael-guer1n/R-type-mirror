/**
 * @file System_ai.hpp
 * @brief Defines AI behavior systems for enemy and boss entities in the server.
 *
 * Uses spell configuration (speed, cooldown, damage, width, height) from JSON files.
 */

#pragma once
#include <cmath>
#include <iostream>
#include <unordered_map>
#include "engine/ecs/Components.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/iterator/Indexed_zipper.hpp"
#include "server/Components_ai.hpp"

using namespace engine;

namespace systems
{
  using AiFunc = std::function<void(entity_t,
                                    registry &,
                                    uint32_t tick,
                                    component::position &,
                                    component::velocity &,
                                    component::ai_controller &)>;

  inline std::unordered_map<std::string, AiFunc> ai_dispatcher;
  inline std::vector<engine::entity_t> spawned_projectiles;

  /**
   * @brief Spawns a projectile with full configurable data.
   */
  inline engine::entity_t server_spawn_projectile(
    engine::registry &r,
    engine::entity_t owner,
    float originX,
    float originY,
    float dirX,
    float dirY,
    float speed,
    int damage,
    float width,
    float height,
    uint32_t lifetime = 300)
{
    auto proj = engine::make_entity(
        r,
        component::position{originX, originY},
        component::hitbox{width, height},
        component::collision_state{false},
        component::entity_kind::enemyProjectile,
        component::projectile_tag{
            static_cast<std::uint32_t>(owner),
            lifetime,
            dirX,
            dirY,
            speed,
            damage},
        component::health{1});
    return proj;
}

  inline void init_ai_behaviors()
{
    // === ENNEMI BASIQUE : avance en ligne droite ===
    ai_dispatcher["basic"] = [](entity_t, registry &, uint32_t, auto &, auto &vel, auto &ai)
    {
        vel.vx = -ai.speed;
        vel.vy = 0.f;
    };

    // === ENNEMI ZIGZAG : mouvement sinusoïdal ===
    ai_dispatcher["zigzag"] = [](entity_t, registry &, uint32_t tick,
                                 auto &, auto &vel, auto &ai)
    {
        vel.vx = -ai.speed;
        vel.vy = std::sin(tick * 0.01f) * ai.speed;
    };

    // === ENNEMI TIREUR DROIT (shooter) ===
    ai_dispatcher["straight_shooter"] = [](entity_t e, registry &r, uint32_t tick,
                                            auto &pos, auto &vel, auto &ai)
    {
        vel.vx = -ai.speed;
        vel.vy = 0.f;

        auto &sbArr = r.get_components<component::spellbook>();
        if (e >= sbArr.size() || !sbArr[e])
            return;

        auto &spells = sbArr[e]->spells;
        for (auto &s : spells)
        {
            if (tick > s.lastCast + s.cooldown)
            {
                s.lastCast = tick;
                float speedVal = std::sqrt(s.speedX * s.speedX + s.speedY * s.speedY);
                auto proj = server_spawn_projectile(
                    r, e, pos.x, pos.y,
                    s.speedX, s.speedY,
                    speedVal, s.damage,
                    s.width, s.height, 300);
                systems::spawned_projectiles.push_back(proj);
            }
        }
    };

    // === ENNEMI TOURNANT (spinner) : tourne autour d’un point imaginaire ===
    ai_dispatcher["spinner"] = [](entity_t, registry &, uint32_t tick,
                                  auto &pos, auto &vel, auto &ai)
    {
        float radius = 2.0f;
        vel.vx = -ai.speed + std::cos(tick * 0.05f) * radius;
        vel.vy = std::sin(tick * 0.05f) * radius;
    };

    // === ENNEMI CHARGER (fonce en ligne droite, s’arrête, repart) ===
    ai_dispatcher["charger"] = [](entity_t, registry &, uint32_t tick,
                                  auto &, auto &vel, auto &ai)
    {
        float phase = std::fmod(tick / 120.0f, 4.0f);
        if (phase < 1.5f)
        {
            vel.vx = -ai.speed * 2.0f; // charge rapide
            vel.vy = 0.f;
        }
        else
        {
            vel.vx = -ai.speed * 0.3f; // ralentit
            vel.vy = 0.f;
        }
    };

    // === ENNEMI BOSS ===
    ai_dispatcher["boss"] = [](entity_t e, registry &r, uint32_t tick,
                                auto &pos, auto &vel, auto &ai)
    {
        vel.vx = -ai.speed;
        vel.vy = std::sin(tick * 0.005f) * ai.speed;

        auto &sbArr = r.get_components<component::spellbook>();
        if (e >= sbArr.size() || !sbArr[e])
            return;

        auto &spells = sbArr[e]->spells;
        auto &positions = r.get_components<component::position>();
        auto &kinds = r.get_components<component::entity_kind>();

        for (auto &s : spells)
        {
            if (tick <= s.lastCast + s.cooldown)
                continue;
            s.lastCast = tick;

            for (size_t i = 0; i < kinds.size(); ++i)
            {
                if (!kinds[i] || kinds[i].value() != component::entity_kind::player)
                    continue;
                if (i >= positions.size() || !positions[i])
                    continue;

                const auto &pPos = positions[i].value();
                float dx = pPos.x - pos.x;
                float dy = pPos.y - pos.y;
                float len = std::sqrt(dx * dx + dy * dy);
                if (len <= 0.001f)
                    continue;
                dx /= len;
                dy /= len;

                float speedVal = std::sqrt(s.speedX * s.speedX + s.speedY * s.speedY);
                auto proj = server_spawn_projectile(
                    r, e, pos.x, pos.y,
                    dx, dy,
                    speedVal, s.damage,
                    s.width, s.height, 300);
                systems::spawned_projectiles.push_back(proj);
            }
        }
    };

/*     ai_dispatcher["boss_laser"] = [](entity_t e, registry &r, uint32_t tick,
                                      auto &pos, auto &vel, auto &ai)
    {
        vel.vx = -ai.speed * 0.5f;
        vel.vy = std::sin(tick * 0.004f) * (ai.speed * 0.8f);

        auto &sbArr = r.get_components<component::spellbook>();
        if (e >= sbArr.size() || !sbArr[e])
            return;

        auto &spells = sbArr[e]->spells;
        for (auto &s : spells)
        {
            if (tick > s.lastCast + s.cooldown)
            {
                s.lastCast = tick;

                std::vector<std::pair<float, float>> dirs = {
                    {-1.f, 0.f},
                    {-0.8f, 0.3f},
                    {-0.8f, -0.3f}};

                for (auto [dx, dy] : dirs)
                {
                    float len = std::sqrt(dx * dx + dy * dy);
                    if (len <= 0.01f)
                        continue;
                    dx /= len;
                    dy /= len;

                    float speedVal = std::sqrt(s.speedX * s.speedX + s.speedY * s.speedY);
                    auto proj = server_spawn_projectile(
                        r, e, pos.x, pos.y, dx, dy, speedVal,
                        s.damage, s.width, s.height, 400);
                    systems::spawned_projectiles.push_back(proj);
                }
            }
        }
    }; */
}

  inline void enemy_ai_system(registry &r,
                              sparse_array<component::position> &positions,
                              sparse_array<component::velocity> &velocities,
                              sparse_array<component::ai_controller> &ais,
                              uint32_t tick)
  {
    for (auto &&[i, pos, vel, ai] :
         indexed_zipper(positions, velocities, ais))
    {
      auto it = ai_dispatcher.find(ai.behavior);
      if (it != ai_dispatcher.end())
        it->second(r.entity_from_index(i), r, tick, pos, vel, ai);
    }
  }

}
