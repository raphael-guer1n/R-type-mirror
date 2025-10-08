#pragma once
#include <memory>
#include "engine/renderer/App.hpp"
#include "engine/renderer/Texture.hpp"
#include "common/Components.hpp"

namespace R_Type
{
    class Rtype;
    class Player {
        public:
            Player(R_Type::Rtype& rtype);
            ~Player() = default;
            std::shared_ptr<engine::R_Graphic::Texture> texture;
            engine::R_Graphic::textureRect playerRect;
            engine::R_Graphic::textureRect projectileRect;
            component::animation playerAnimation;
        public:
            void playerUpdateAnimation(std::unordered_map<uint32_t, size_t>& entityMap,
                uint32_t player, engine::registry& registry, uint8_t keys);
    };
}
