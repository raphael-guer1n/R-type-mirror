#pragma once
#include <memory>
#include "R_Graphic/Texture.hpp"
#include "R_Graphic/App.hpp"
#include "R_Ecs/Registry.hpp"
#include "R_Ecs/Sparse_array.hpp"
#include "R_Ecs/Components.hpp"

namespace R_Ecs
{
    namespace Component
    {
        struct background_tag {};
    }
}
namespace R_Type
{
    class Rtype;
    void scroll_reset_system(R_Ecs::Registry& r,
    R_Ecs::Sparse_array<R_Ecs::Component::position> &positions,
    R_Ecs::Sparse_array<R_Ecs::Component::background_tag> &backgrounds,
    R_Graphic::App &app);
    class Background
    {
        public:
            Background(R_Type::Rtype& rtype);
            ~Background() = default;
            void update(R_Graphic::App& app, R_Ecs::Registry& reg, float deltaTime);
            void draw(R_Graphic::App& app, R_Ecs::Registry& reg);
    };
}
