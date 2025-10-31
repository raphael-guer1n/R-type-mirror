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

    class Enemy {
    public:
        Enemy(R_Type::Rtype& rtype);
        ~Enemy() = default;

        std::shared_ptr<engine::R_Graphic::Texture> enemyTexture;
        std::shared_ptr<engine::R_Graphic::Texture> projectileTexture;

        engine::R_Graphic::textureRect enemyRect;
        engine::R_Graphic::textureRect projectileRect;

        component::animation projectileAnimation;

        void setType(const std::string& type);
        std::string getType() const { return _enemyType; }

    private:
        R_Type::Rtype& _rtypeRef;
        std::string _enemyType;
    };
}