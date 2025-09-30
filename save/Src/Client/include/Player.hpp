#pragma once
#include <memory>
#include "R_Graphic/Texture.hpp"
#include "R_Graphic/App.hpp"
#include "R_Ecs/Registry.hpp"

namespace R_Type
{
    class Rtype;
    class Player {
        public:
            Player(R_Type::Rtype& rtype);
            ~Player() = default;
    };
}
