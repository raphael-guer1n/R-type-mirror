#include "common/Components.hpp"
#include "engine/ecs/EntityFactory.hpp"

engine::entity_t spawn_projectile_basic(engine::entity_t owner, engine::registry &reg)
{
    auto &positions = reg.get_components<component::position>();
    auto &hitboxes = reg.get_components<component::hitbox>();
    size_t idx = static_cast<size_t>(owner);
    if (idx >= positions.size() || !positions[idx])
        return owner;
    auto pos = positions[idx].value();
    float playerW = 0.f, playerH = 0.f;
    if (idx < hitboxes.size() && hitboxes[idx])
    {
        playerW = hitboxes[idx]->width;
        playerH = hitboxes[idx]->height;
    }
    constexpr float w = 10.f, h = 10.f;
    float startX = pos.x + playerW + 4.f;
    float startY = pos.y + (playerH * 0.5f) - (h * 0.5f);
    return engine::make_entity(
        reg,
        component::position{startX, startY}, component::velocity{1.f, 0.f},
        component::hitbox{w, h},
        component::collision_state{false},
        component::entity_kind::playerProjectile,
        component::projectile_tag{static_cast<uint32_t>(owner), 120, 1.f, 0.f, 2.0f, 2},
        component::health{1}
    );
}

engine::entity_t spawn_projectile_alt(engine::entity_t owner, engine::registry &reg)
{
    auto &positions = reg.get_components<component::position>();
    auto &hitboxes = reg.get_components<component::hitbox>();
    size_t idx = static_cast<size_t>(owner);
    if (idx >= positions.size() || !positions[idx])
        return owner;
    auto pos = positions[idx].value();
    float playerW = 0.f, playerH = 0.f;
    if (idx < hitboxes.size() && hitboxes[idx])
    {
        playerW = hitboxes[idx]->width;
        playerH = hitboxes[idx]->height;
    }
    constexpr float w = 12.f, h = 12.f;
    float startX = pos.x + playerW + 4.f;
    float startY = pos.y + (playerH * 0.5f) - (h * 0.5f);
    return engine::make_entity(
        reg,
        component::position{startX, startY}, component::velocity{1.f, 0.f},
        component::hitbox{w, h},
        component::collision_state{false},
        component::entity_kind::playerProjectile,
        component::projectile_tag{static_cast<uint32_t>(owner), 120, 1.f, 0.f, 3.0f, 2},
        component::health{1}
    );
}

engine::entity_t spawn_projectile_charged(engine::entity_t owner, uint32_t heldTicks, engine::registry &reg)
{
    auto &positions = reg.get_components<component::position>();
    auto &hitboxes = reg.get_components<component::hitbox>();
    size_t idx = static_cast<size_t>(owner);
    if (idx >= positions.size() || !positions[idx])
        return owner;
    auto pos = positions[idx].value();
    float playerW = 0.f, playerH = 0.f;
    if (idx < hitboxes.size() && hitboxes[idx]) {
        playerW = hitboxes[idx]->width;
        playerH = hitboxes[idx]->height;
    }
    float scale = std::min(1.0f + (heldTicks / 60.0f), 3.0f);
    float w = 14.f * scale, h = 14.f * scale;
    float speed = 5.5f + 1.0f * scale;
    int dmg = static_cast<int>(2 * scale) + 1;
    float startX = pos.x + playerW + 4.f;
    float startY = pos.y + (playerH * 0.5f) - (h * 0.5f);
    return engine::make_entity(
        reg,
    component::position{startX, startY}, component::velocity{speed, 0.f},
        component::hitbox{w, h},
        component::collision_state{false},
        component::entity_kind::projectile_charged,
        component::projectile_tag{static_cast<uint32_t>(owner), 180, 1.f, 0.f, speed, dmg},
        component::health{1}
    );
}

engine::entity_t spawn_projectile_bomb(engine::entity_t owner, engine::registry &reg)
{
    auto &positions = reg.get_components<component::position>();
    auto &hitboxes = reg.get_components<component::hitbox>();
    size_t idx = static_cast<size_t>(owner);
    if (idx >= positions.size() || !positions[idx])
        return owner;
    auto pos = positions[idx].value();
    float playerW = 0.f, playerH = 0.f;
    if (idx < hitboxes.size() && hitboxes[idx])
    {
        playerW = hitboxes[idx]->width;
        playerH = hitboxes[idx]->height;
    }

    constexpr float w = 17.f, h = 17.f;
    float startX = pos.x + playerW - w * 0.2f;
    float startY = pos.y + (playerH * 0.5f) - (h * 0.5f);

    float dirX = 0.8f;
    float dirY = -0.9f;
    float speed = 2.2f;
    uint32_t lifetime = 240;
    int damage = 3;

    return engine::make_entity(
        reg,
        component::position{startX, startY}, component::velocity{dirX * speed, dirY * speed},
        component::hitbox{w, h},
        component::collision_state{false},
        component::entity_kind::projectile_bomb,
        component::projectile_tag{static_cast<uint32_t>(owner), lifetime, dirX, dirY, speed, damage},
        component::gravity{0.03f},
        component::health{1}
    );
}

engine::entity_t spawn_projectile(engine::entity_t owner, engine::registry &reg)
{
    auto &positions = reg.get_components<component::position>();
    auto &hitboxes = reg.get_components<component::hitbox>();
    size_t idx = static_cast<size_t>(owner);
    if (idx >= positions.size() || !positions[idx])
        return owner;
    auto pos = positions[idx].value();
    float playerW = 0.f, playerH = 0.f;
    if (idx < hitboxes.size() && hitboxes[idx])
    {
        playerW = hitboxes[idx]->width;
        playerH = hitboxes[idx]->height;
    }
    constexpr float projectileW = 72.f;
    constexpr float projectileH = 24.f;
    float startX = pos.x + playerW + 4.f;
    float startY = pos.y + (playerH * 0.5f) - (projectileH * 0.5f);
    auto proj = engine::make_entity(
        reg,
    component::position{startX, startY}, component::velocity{2.f, 0.f},
        component::hitbox{projectileW, projectileH},
        component::collision_state{false},
        component::entity_kind::playerProjectile,
        component::projectile_tag{static_cast<uint32_t>(owner), 120, 1.f, 0.f, 2.f, 2},
        component::health{1});
    return proj;
}

engine::entity_t spawn_missile_explosion(float x, float y, int damage, float radius, engine::registry &reg)
{
    float size = radius * 2.f;
    float topLeftX = x - radius;
    float topLeftY = y - radius;
    auto e = engine::make_entity(
        reg,
        component::position{topLeftX, topLeftY},
        component::velocity{0.f, 0.f},
        component::hitbox{size, size},
        component::collision_state{false},
        component::entity_kind::missile_explosion,
        component::projectile_tag{0u, 30u, 0.f, 0.f, 0.f, damage}, // ~0.5s lifetime to match animation
        component::area_effect{radius, damage, false},
        component::health{1});
    return e;
}
