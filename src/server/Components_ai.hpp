/**
 * @file Components_ai.hpp
 * @brief Defines AI-related components for the server, including controllers, spells, spellbooks, and boss phases.
 */

#pragma once
#include <string>
#include <vector>
#include <cstdint>

    /**
     * @struct ai_controller
     * @brief Represents an AI controller for an entity, specifying behavior and action parameters.
     *
     * @var ai_controller::behavior
     *   The behavior pattern or type for the AI.
     * @var ai_controller::speed
     *   Movement speed of the AI entity.
     * @var ai_controller::shootCooldown
     *   Cooldown period (in ticks) between shooting actions.
     * @var ai_controller::lastActionTick
     *   The tick count of the last performed action.
     */

    /**
     * @struct spell
     * @brief Represents a spell that can be cast by an entity.
     *
     * @var spell::name
     *   Name of the spell.
     * @var spell::speedX
     *   Horizontal speed of the spell projectile.
     * @var spell::speedY
     *   Vertical speed of the spell projectile.
     * @var spell::cooldown
     *   Cooldown period (in ticks) between casts.
     * @var spell::lastCast
     *   The tick count of the last cast.
     * @var spell::damage
     *   Damage dealt by the spell.
     * @var spell::width
     *   Width of the spell projectile.
     * @var spell::height
     *   Height of the spell projectile.
     */

    /**
     * @struct spellbook
     * @brief Contains a collection of spells available to an entity.
     *
     * @var spellbook::spells
     *   List of spells in the spellbook.
     */

    /**
     * @struct boss_phase
     * @brief Represents a phase in a boss fight, triggered by HP threshold.
     *
     * @var boss_phase::hpThreshold
     *   HP value at which the phase is triggered.
     * @var boss_phase::nextAI
     *   Name of the next AI behavior to switch to.
     */
namespace component
{
    struct ai_controller {
        std::string behavior;
        float speed = 2.0f;
        uint32_t shootCooldown = 0;
        uint32_t lastActionTick = 0;
    };

    struct spell {
        std::string name;
        float speedX = 0, speedY = 0;
        uint32_t cooldown = 60;
        uint32_t lastCast = 0;
        int damage = 1;
        float width = 10.f, height = 10.f;
    };

    struct spellbook {
        std::vector<spell> spells;
    };

    struct boss_phase {
        int hpThreshold{0};
        std::string nextAI;
    };
}
