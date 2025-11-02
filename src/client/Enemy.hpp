/**
 * @file Enemy.hpp
 * @brief Declaration of the Enemy class representing an enemy entity in the R-Type game.
 *
 * This class manages enemy properties, textures, animations, and type within the R-Type game engine.
 * It interacts with the main game engine through a reference to the @ref R_Type::Rtype object.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_set>
#include "engine/renderer/App.hpp"
#include "engine/renderer/Texture.hpp"
#include "engine/ecs/Components.hpp"
#include "engine/events/Events.hpp"

namespace R_Type
{
    class Rtype;

    /**
     * @class Enemy
     * @brief Represents an enemy entity in the R-Type game.
     *
     * The Enemy class encapsulates all data required to manage an enemy, including textures,
     * animations, and type. It also allows loading specific assets depending on the enemy type
     * (e.g., `"basic"`, `"boss"`, `"turret"`, etc.).
     *
     * @note Each Enemy instance holds a reference to the main game engine (@ref R_Type::Rtype),
     * allowing it to interact with the ECS system and the engine’s event manager.
     */
    class Enemy {
    public:
        /**
         * @brief Constructs an Enemy object.
         *
         * Initializes an enemy entity and links it to the main Rtype engine instance.
         *
         * @param rtype Reference to the main Rtype engine object.
         */
        Enemy(R_Type::Rtype& rtype);

        /**
         * @brief Default destructor for the Enemy class.
         */
        ~Enemy() = default;

        /**
         * @brief Main texture representing the enemy sprite.
         */
        std::shared_ptr<engine::R_Graphic::Texture> enemyTexture;

        /**
         * @brief Texture used for the enemy's projectiles.
         */
        std::shared_ptr<engine::R_Graphic::Texture> projectileTexture;

        /**
         * @brief Texture rectangle defining the sprite region for the enemy.
         */
        engine::R_Graphic::textureRect enemyRect;

        /**
         * @brief Texture rectangle defining the sprite region for the projectile.
         */
        engine::R_Graphic::textureRect projectileRect;

        /**
         * @brief Animation data for the enemy’s projectile.
         */
        component::animation projectileAnimation;

        /**
         * @brief Sets the type of the enemy.
         *
         * The type determines its behavior, appearance, and attack pattern.
         *
         * @param type String representing the enemy type (e.g., `"basic"`, `"boss"`, `"turret"`).
         */
        void setType(const std::string& type);

        /**
         * @brief Gets the current enemy type.
         *
         * @return The enemy type as a string.
         */
        std::string getType() const { return _enemyType; }

    private:
        /**
         * @brief Reference to the main Rtype game engine instance.
         */
        R_Type::Rtype& _rtypeRef;

        /**
         * @brief Enemy type identifier (e.g., `"basic"`, `"boss"`, `"turret"`).
         */
        std::string _enemyType;
    };
}
