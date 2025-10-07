#pragma once
#include "common/Components.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/iterator/Indexed_zipper.hpp"
#include "server/Components_ai.hpp"
#include <cmath>
#include <iostream>
#include <unordered_map>

using namespace engine;

namespace systems {
using AiFunc = std::function<void(entity_t, registry &, uint32_t tick,
                                  component::position &, component::velocity &,
                                  component::ai_controller &)>;

inline std::unordered_map<std::string, AiFunc> ai_dispatcher;

inline void init_ai_behaviors() {
  // Simple left-mover
  ai_dispatcher["basic"] = [](entity_t, registry &, uint32_t, auto &pos,
                              auto &vel, auto &ai) {
    vel.vx = -ai.speed;
    vel.vy = 0.f;
  };

  // Zigzag pattern
  ai_dispatcher["zigzag"] = [](entity_t, registry &, uint32_t tick, auto &pos,
                               auto &vel, auto &ai) {
    vel.vx = -ai.speed;
    vel.vy = std::sin(tick * 0.01f) * ai.speed;
  };

  ai_dispatcher["straight_shooter"] = [](entity_t e, registry &r, uint32_t tick,
                                         auto &pos, auto &vel, auto &ai) {
    vel.vx = -ai.speed;
    vel.vy = 0.f;

    auto &sbArr = r.get_components<component::spellbook>();
    if (e < sbArr.size() && sbArr[e]) {
      for (auto &s : sbArr[e]->spells) {
        if (tick > s.lastCast + s.cooldown) {
          s.lastCast = tick;
          auto bullet = r.spawn_entity();
          r.add_component(bullet, component::position{pos.x, pos.y});
          r.add_component(bullet, component::velocity{s.speedX, s.speedY});
          r.add_component(bullet, component::hitbox{s.width, s.height});
          r.add_component(bullet, component::entity_kind::projectile);
          r.add_component(bullet, component::damage{s.damage});
        }
      }
    }
  };
  // Boss Phase 1
  ai_dispatcher["boss_phase1"] = [](entity_t e, registry &r, uint32_t tick,
                                    auto &pos, auto &vel, auto &ai) {
    vel.vx = -1.0f;
    vel.vy = std::sin(tick * 0.05f) * 4.0f;

    auto &spells = r.get_components<component::spellbook>()[e];
    if (spells) {
      for (auto &s : spells->spells) {
        if (s.name == "fireball" && tick > s.lastCast + s.cooldown) {
          s.lastCast = tick;
          auto bullet = r.spawn_entity();
          r.add_component(bullet, component::position{pos.x, pos.y});
          r.add_component(bullet, component::velocity{s.speedX, s.speedY});
          r.add_component(bullet, component::hitbox{s.width, s.height});
          r.add_component(bullet, component::entity_kind::projectile);
          r.add_component(bullet, component::damage{s.damage});
          std::cout << "Boss cast fireball!\n";
        }
      }
    }
  };

  // Boss Phase 2
  ai_dispatcher["boss_phase2"] = [](entity_t e, registry &r, uint32_t tick,
                                    auto &pos, auto &vel, auto &ai) {
    vel.vx = -0.5f;
    vel.vy = std::cos(tick * 0.1f) * 6.0f;

    auto &spells = r.get_components<component::spellbook>()[e];
    if (spells) {
      for (auto &s : spells->spells) {
        if (s.name == "spreadshot" && tick > s.lastCast + s.cooldown) {
          s.lastCast = tick;
          // Fire 3 projectiles spread
          for (int dy = -1; dy <= 1; ++dy) {
            auto bullet = r.spawn_entity();
            r.add_component(bullet, component::position{pos.x, pos.y});
            r.add_component(bullet,
                            component::velocity{s.speedX, s.speedY + dy * 2});
            r.add_component(bullet, component::hitbox{s.width, s.height});
            r.add_component(bullet, component::entity_kind::projectile);
            r.add_component(bullet, component::damage{s.damage});
          }
          std::cout << "Boss cast spreadshot!\n";
        }
      }
    }
  };
}

inline void enemy_ai_system(registry &r,
                            sparse_array<component::position> &positions,
                            sparse_array<component::velocity> &velocities,
                            sparse_array<component::ai_controller> &ais,
                            uint32_t tick) {
  for (auto &&[i, pos, vel, ai] : indexed_zipper(positions, velocities, ais)) {
    auto it = ai_dispatcher.find(ai.behavior);
    if (it != ai_dispatcher.end()) {
      it->second(r.entity_from_index(i), r, tick, pos, vel, ai);
    }
  }
}

inline void boss_phase_system(registry &r,
                              sparse_array<component::health> &healths,
                              sparse_array<component::ai_controller> &ai,
                              sparse_array<component::boss_phase> &phases) {
  for (auto &&[i, h, ctrl, phase] : indexed_zipper(healths, ai, phases)) {
    if (h.hp <= phase.hpThreshold && ctrl.behavior != phase.nextAI) {
      ctrl.behavior = phase.nextAI;
      std::cout << "Boss switched to " << phase.nextAI << "!\n";
    }
  }
}

} // namespace systems