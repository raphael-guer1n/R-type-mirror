#pragma once
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Registry.hpp"

engine::entity_t spawn_projectile_basic(engine::entity_t owner,
    engine::registry& reg);

engine::entity_t spawn_projectile_alt(engine::entity_t owner,
    engine::registry& reg);

engine::entity_t spawn_projectile_charged(engine::entity_t owner,
    uint32_t heldTicks, engine::registry& reg);

engine::entity_t spawn_projectile_bomb(engine::entity_t owner,
    engine::registry& reg);

engine::entity_t spawn_projectile(engine::entity_t owner,
    engine::registry& reg);

engine::entity_t spawn_missile_explosion(float x, float y, int damage,
    float radius, engine::registry& reg);