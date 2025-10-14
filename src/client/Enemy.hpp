#pragma once
#include <memory>
#include <unordered_set>
#include "engine/renderer/App.hpp"
#include "engine/renderer/Texture.hpp"
#include "common/Components.hpp"
#include "engine/events/Events.hpp"

namespace R_Type
{
    class Rtype;
    class Enemy {
        public:
            Enemy(R_Type::Rtype& rtype);
            ~Enemy() = default;

            std::shared_ptr<engine::R_Graphic::Texture> enemyTexture;
            std::shared_ptr<engine::R_Graphic::Texture> projectileTexture;
            engine::R_Graphic::textureRect enemyRect;
            engine::R_Graphic::textureRect projectileRect;
            component::animation projectileAnimation;
        public:
            // void enemyUpdateAnimation(std::unordered_map<uint32_t, size_t>& entityMap,
            //     uint32_t player, engine::registry& registry, const std::unordered_set<engine::R_Events::Key>& pressedKeys);
    };
}