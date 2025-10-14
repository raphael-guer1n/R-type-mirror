#pragma once
#include <memory>
#include <optional>
#include "engine/renderer/App.hpp"
#include "engine/renderer/Texture.hpp"
#include "common/Components.hpp"
#include "engine/events/Events.hpp"
/**
 * @file Player.hpp
 * @brief Defines the Player class for the R-Type client.
 *
 * This header declares the Player class, which manages the player's graphical representation,
 * animations, and interactions within the R-Type game client. The Player class encapsulates
 * texture handling, animation states for both the player and projectiles, and provides
 * functionality to update the player's animation based on input events.
 *
 * @author marysekatary
 * @date 2024
 */

namespace R_Type
{
    /**
     * @class Player
     * @brief Represents a player entity in the R-Type client.
     *
     * The Player class is responsible for managing the player's texture, animation states,
     * and updating the player's animation according to user input. It interacts with the
     * main Rtype game instance and the rendering engine to display and animate the player.
     */
    class Rtype;
    class Player {
        public:
                    /**
             * @brief Constructs a Player object.
             * @param rtype Reference to the main Rtype game instance.
             */
            Player(R_Type::Rtype& rtype);
                        /**
             * @brief Default destructor.
             */
            ~Player() = default;

            /**
             * @brief Updates the player's animation based on pressed keys and entity mapping.
             * @param entityMap Mapping of entity IDs to their indices.
             * @param player The unique identifier for the player entity.
             * @param registry Reference to the entity-component registry.
             * @param pressedKeys Set of currently pressed keys.
             */
            
            std::shared_ptr<engine::R_Graphic::Texture> playerTexture;
            std::shared_ptr<engine::R_Graphic::Texture> projectileTexture;
            engine::R_Graphic::textureRect playerRect;
            engine::R_Graphic::textureRect projectileRect;
            std::shared_ptr<engine::R_Graphic::Texture> chargeTexture;
            std::shared_ptr<engine::R_Graphic::Texture> chargeProjectileTexture;
            std::shared_ptr<engine::R_Graphic::Texture> missileProjectileTexture;
            std::shared_ptr<engine::R_Graphic::Texture> missileExplosionTexture;
            engine::R_Graphic::textureRect chargeRect;
            engine::R_Graphic::textureRect chargeProjectileRect;
            engine::R_Graphic::textureRect missileProjectileRect;
            engine::R_Graphic::textureRect missileexplosionRect;
            component::animation playerAnimation;
            component::animation projectileAnimation;
            component::animation chargeAnimation;
            component::animation chargeProjectileAnimation;
            component::animation missileProjectileAnimation;
            component::animation missileexplosionAnimation;
            std::optional<size_t> chargeOverlayLocalId;
        public:
            void playerUpdateAnimation(std::unordered_map<uint32_t, size_t>& entityMap,
                uint32_t player, engine::registry& registry, const std::unordered_set<engine::R_Events::Key>& pressedKeys);
        private:
            void ensureChargeOverlay(engine::registry& registry, size_t playerLocalId, bool show);
            void updateChargeOverlayPosition(engine::registry& registry, size_t playerLocalId);
    };
}
