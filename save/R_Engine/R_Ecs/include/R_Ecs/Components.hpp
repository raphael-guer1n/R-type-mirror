#pragma once
#include <memory>
#include "R_Graphic/Texture.hpp"

namespace R_Ecs
{
    namespace Component
    {
        // Position 2D
        struct position
        {
            float x{}, y{};
            position() = default;
            position(float x_, float y_) : x(x_), y(y_) {}
        };

        // Vitesse 2D
        struct velocity
        {
            float vx{}, vy{};
            velocity() = default;
            velocity(float vx_, float vy_) : vx(vx_), vy(vy_) {}
        };

        // Components.hpp
        struct drawable
        {
            R_Graphic::textureRect rect;
            std::shared_ptr<R_Graphic::Texture> texture;

            drawable() = default;
            drawable(std::shared_ptr<R_Graphic::Texture> tex,
             R_Graphic::textureRect r = R_Graphic::textureRect()) : rect(r), texture(std::move(tex)) {}
        };
        // Marqueur de contrÃ´le clavier
        struct controllable
        {
            int inputX = 0; // -1 = left, +1 = right
            int inputY = 0; // -1 = up, +1 = down
            bool shoot = false;
            bool enabled = true;
        };
        struct player_tag {};

    }
}