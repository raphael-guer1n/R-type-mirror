#pragma once
#include <memory>
#include "Player.hpp"
#include "R_Graphic/App.hpp"
#include "R_Ecs/Registry.hpp"
#include "Background.hpp"

namespace R_Type
{
    class Rtype
    {
        public:
            Rtype();
            ~Rtype() = default;
            void update(float deltaTime);
            void draw();
            R_Graphic::App& getApp();
            R_Ecs::Registry& getRegistry();
        private:
            R_Graphic::App _app;
            R_Ecs::Registry _registry;
            std::unique_ptr<Background> _background;
            std::unique_ptr<Player> _player;
    };
}
