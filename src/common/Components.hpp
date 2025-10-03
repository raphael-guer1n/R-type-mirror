#pragma once
#include <cstdint>
#include <functional>

// Forward declarations to allow spawn_request factory signature without including Registry.hpp
namespace engine
{
    class registry;
    class entity_t;
}
/**
    * @file Components.hpp
    * @brief Common ECS components used by both client and server.
    */

namespace component
{
/**
    * @brief Different kinds of entities in the game.
    */
    enum class entity_kind : std::uint8_t
    {
        unknown = 0,
        player = 1,
        enemy = 2,
        projectile = 3,
        pickup = 4,
        decor = 5
    };
/**
    * @brief Basic 2D position component.
    */
    struct position
    {
        float x{}, y{};
        position() = default;
        position(float x_, float y_) : x(x_), y(y_) {}
    };

/**
    * @brief Basic 2D velocity component.
    */
    struct velocity
    {
        float vx{}, vy{};
        velocity() = default;
        velocity(float vx_, float vy_) : vx(vx_), vy(vy_) {}
    };

/**
    * @brief Drawable component for rendering entities.
    */
    struct controllable
    {
        int inputX = 0; // -1 = left, +1 = right
        int inputY = 0; // -1 = up, +1 = down
        bool shoot = false;
    };
/**
    * @brief Simple AABB hitbox component.
    */
    struct hitbox
    {
        float width{}, height{};
        float offset_x{};
        float offset_y{};
        hitbox() = default;
        hitbox(float w, float h, float ox = 0.f, float oy = 0.f) : width(w), height(h), offset_x(ox), offset_y(oy) {}
    };
/**
    * @brief Health component for entities.
    */  
    struct health
    {
        std::uint8_t hp{1};
        health() = default;
        explicit health(std::uint8_t h) : hp(h) {}
    };
/**
    * @brief Simple drawable component for rendering entities.  
    */
    struct net_id
    {
        std::uint32_t id{0};
        std::uint8_t type{0}; // deprecated, prefer entity_kind
    };
/**
    * @brief Simple drawable component for rendering entities.
    */
    struct collision_state
    {
        bool collided{false};
    };
/**
    * @brief Component to mark which player controls an entity.
    */
    struct controlled_by
    {
        std::uint32_t owner{0};
    };
/**
    * @brief Simple drawable component for rendering entities.
    */
    struct damage
    {
        int amount{0};
    };
/**
    * @brief Tag component to mark an entity for despawning.
    */
    struct despawn_tag
    {
        bool now{true};
    };
/**
    * @brief Component to request spawning of an entity with a factory function.
    */
    struct spawn_request
    {
        std::function<void(engine::registry &, engine::entity_t)> factory;
    };
/**
    * @brief Damage cooldown to prevent rapid repeated damage.
    */
    struct damage_cooldown
    {
        uint32_t last_hit_tick{0};
    };

} // namespace component