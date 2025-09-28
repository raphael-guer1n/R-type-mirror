#pragma once

#include <SFML/Graphics.hpp>
#include "R_Graphic/Window.hpp"
#include "R_Graphic/Texture.hpp"
#include "Registry.hpp"
#include "Components.hpp"
#include "iterator/Zipper.hpp"
#include "iterator/Indexed_zipper.hpp"

namespace R_Ecs
{
    void position_system(Registry &r, Sparse_array<Component::position> &positions,
        Sparse_array<Component::velocity> &velocities, float deltaTime)
    {
        for (auto &&[pos, vel] : Zipper(positions, velocities))
        {
            pos.x += vel.vx * deltaTime;
            pos.y += vel.vy * deltaTime;
        }
    }

    void control_system(Registry &r,
        Sparse_array<Component::velocity> &velocities,
        Sparse_array<Component::controllable> &controls)
    {
        constexpr float speed = 5.f;

        for (auto &&[vel, c] : Zipper(velocities, controls))
        {
            vel.vx = c.inputX * speed;
            vel.vy = c.inputY * speed;
            // shooting can be handled separately
        }
    }

    void draw_system(Registry &r,
        Sparse_array<Component::position> &positions,
        Sparse_array<Component::drawable> &drawables,
        R_Graphic::Window &window)
    {
        for (auto &&[pos, dr] : Zipper(positions, drawables))
        {
            if (dr.texture) {
                dr.texture->position = {pos.x, pos.y};
                dr.texture->draw(window, nullptr);
            }
        }
    }
}
