#pragma once
#include "engine/ecs/Registry.hpp"
#include "common/Components.hpp"
#include "engine/ecs/iterator/Zipper.hpp"
#include "engine/ecs/iterator/Indexed_zipper.hpp"
#include <algorithm>
#include <vector>

using namespace engine;
/**
 * @file Systems.hpp
 * @brief Defines core ECS systems for the game engine.
 *
 * This file implements several systems operating on ECS components:
 * - position_system: Updates entity positions based on velocity.
 * - hitbox_system: Detects collisions between entities and triggers a callback.
 * - health_system: Applies damage, updates health, and marks entities for despawn if health reaches zero.
 * - despawn_system: Removes entities marked for despawn.
 * - spawn_system: Handles entity spawning via factory callbacks.
 *
 * Each system operates on sparse arrays of components and interacts with the registry.
 */
void position_system(registry &r,
                     sparse_array<component::position> &positions,
                     sparse_array<component::velocity> &velocities)
{
    for (auto &&[i, pos, vel] : indexed_zipper(positions, velocities))
    {
        pos.x += vel.vx;
        pos.y += vel.vy;
    }
}

template <typename Callback>
void hitbox_system(registry &r,
                   sparse_array<component::position> &positions,
                   sparse_array<component::hitbox> &hitboxes,
                   Callback on_collision)
{
    for (auto &&[i, posA, hbA] : indexed_zipper(positions, hitboxes))
    {
        for (std::size_t j = i + 1; j < positions.size(); ++j)
        {
            if (!positions[j] || !hitboxes[j])
                continue;
            const auto &posB = positions[j].value();
            const auto &hbB = hitboxes[j].value();
            if (posA.x + hbA.offset_x < posB.x + hbB.offset_x + hbB.width &&
                posA.x + hbA.offset_x + hbA.width > posB.x + hbB.offset_x &&
                posA.y + hbA.offset_y < posB.y + hbB.offset_y + hbB.height &&
                posA.y + hbA.offset_y + hbA.height > posB.y + hbB.offset_y)
            {
                on_collision(i, j);
            }
        }
    }
}

// Apply damage to health and mark entities for despawn when hp <= 0
inline void health_system(registry &r,
                          sparse_array<component::health> &healths,
                          sparse_array<component::damage> &damages)
{
    std::vector<entity_t> toKill;
    for (auto &&[i, h] : indexed_zipper(healths))
    {
        if (i < damages.size() && damages[i] && damages[i].value().amount != 0)
        {
            int hp = static_cast<int>(h.hp);
            hp -= damages[i].value().amount;
            h.hp = static_cast<std::uint8_t>(std::max(0, hp));
            damages[i].value().amount = 0;
        }
        if (h.hp == 0)
            toKill.push_back(r.entity_from_index(i));
    }
    for (auto e : toKill) r.kill_entity(e);
}

// Handle spawn requests via factory function
inline void spawn_system(registry &r,
                         sparse_array<component::spawn_request> &spawns)
{
    for (auto &&[i, req] : indexed_zipper(spawns))
    {
        auto copy = req;          // copy, as we will clear the slot
        spawns[i] = std::nullopt; // consume request
        auto e = r.spawn_entity();
        if (copy.factory)
            copy.factory(r, e);
    }
}