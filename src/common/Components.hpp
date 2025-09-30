#pragma once
#include <cstdint>
#include <functional>

namespace engine
{
    class registry;
    class entity_t;
}

namespace component
{

    enum class entity_kind : std::uint8_t
    {
        unknown = 0,
        player = 1,
        enemy = 2,
        projectile = 3,
        pickup = 4,
        decor = 5
    };

    struct position
    {
        float x{}, y{};
        position() = default;
        position(float x_, float y_) : x(x_), y(y_) {}
    };

    struct velocity
    {
        float vx{}, vy{};
        velocity() = default;
        velocity(float vx_, float vy_) : vx(vx_), vy(vy_) {}
    };

    struct controllable
    {
        int inputX = 0;
        int inputY = 0;
        bool shoot = false;
    };

    struct hitbox
    {
        float width{}, height{};
        float offset_x{};
        float offset_y{};
        hitbox() = default;
        hitbox(float w, float h, float ox = 0.f, float oy = 0.f) : width(w), height(h), offset_x(ox), offset_y(oy) {}
    };

    struct health
    {
        std::uint8_t hp{1};
        health() = default;
        explicit health(std::uint8_t h) : hp(h) {}
    };

    struct net_id
    {
        std::uint32_t id{0};
        std::uint8_t type{0};
    };

    struct collision_state
    {
        bool collided{false};
    };

    struct controlled_by
    {
        std::uint32_t owner{0};
    };

    struct damage
    {
        int amount{0};
    };

    struct despawn_tag
    {
        bool now{true};
    };

    struct spawn_request
    {
        std::function<void(engine::registry &, engine::entity_t)> factory;
    };

    struct damage_cooldown
    {
        uint32_t last_hit_tick{0};
    };

} // namespace component