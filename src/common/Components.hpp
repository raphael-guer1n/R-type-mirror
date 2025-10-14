#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include <functional>

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
        projectile_bomb = 7,
        projectile_charged = 8,
        pickup = 4,
        decor = 5,
        explosion = 9
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
        int inputX = 0;
        int inputY = 0;
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
        std::uint8_t type{0};
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

    struct projectile_tag
    {
        std::uint32_t owner{0};
        std::uint32_t lifetime{180};
        float dirX{1.f};
        float dirY{0.f};
        float speed{20.f};
        int damage{1};
    };

    // Simple gravity acceleration for ballistic movement (adds to projectile_tag.dirY each tick)
    struct gravity
    {
        float ay{0.03f};
    };

    // Area-of-effect applied once on spawn (or first tick) for explosion entities
    struct area_effect
    {
        float radius{100.f};
        int damage{1};
        bool applied{false};
    };

    struct AnimationClip
    {
        int frameCount = 1;
        float frameTime = 0.1f;
        int startX = 0;
        int startY = 0;
        int frameWidth = 0;
        int frameHeight = 0;
        bool loop = false;
    };

    struct animation
    {
        std::unordered_map<std::string, AnimationClip> clips;
        std::string currentClip = "idle";
        int currentFrame = 0;
        float timer = 0.f;
        bool reverse = false;
    };

} // namespace component
