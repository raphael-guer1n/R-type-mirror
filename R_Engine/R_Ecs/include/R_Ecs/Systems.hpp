#pragma once

#include "R_Graphic/Window.hpp"
#include "R_Graphic/Texture.hpp"
#include "Registry.hpp"
#include "Components.hpp"
#include "iterator/Zipper.hpp"
#include "iterator/Indexed_zipper.hpp"

namespace R_Ecs
{
    inline void position_system(Registry &r, Sparse_array<Component::position> &positions,
        Sparse_array<Component::velocity> &velocities, float deltaTime)
    {
        for (auto &&[pos, vel] : Zipper(positions, velocities))
        {
            pos.x += vel.vx * deltaTime;
            pos.y += vel.vy * deltaTime;
        }
    }

    inline void boundary_system(Registry &, Sparse_array<Component::position> &positions,
        Sparse_array<Component::drawable> &drawables,Sparse_array<Component::player_tag> &players,
        R_Graphic::Window &window)
    {
        int winW = window.getSize().x;
        int winH = window.getSize().y;

        for (size_t i = 0; i < positions.size(); ++i) {
            if (!positions[i].has_value() || !drawables[i].has_value() || !players[i].has_value())
                continue;

            auto &pos = positions[i].value();
            auto &dr = drawables[i].value();
            int spriteW = dr.rect.size.x;
            int spriteH = dr.rect.size.y;
            
            if (pos.x < 0)
                pos.x = 0;
            if (pos.y < 0)
                pos.y = 0;
            if (pos.x + spriteW > winW)
                pos.x = spriteW - winW ;
            if (pos.y + spriteH > winH)
                pos.y = spriteH - winH;
        }
    }


    inline void input_system(Registry &,
        Sparse_array<Component::controllable> &controls)
    {
        const Uint8* state = SDL_GetKeyboardState(nullptr);

        for (auto &&[c] : Zipper(controls))
        {
            c.inputX = 0;
            c.inputY = 0;

            if (state[SDL_SCANCODE_LEFT])  c.inputX = -1;
            if (state[SDL_SCANCODE_RIGHT]) c.inputX =  1;
            if (state[SDL_SCANCODE_UP])    c.inputY = -1;
            if (state[SDL_SCANCODE_DOWN])  c.inputY =  1;

            c.shoot = state[SDL_SCANCODE_SPACE];
        }
    }

    inline void control_system(Registry &r,
        Sparse_array<Component::velocity> &velocities,
        Sparse_array<Component::controllable> &controls)
    {
        constexpr float speed = 200.f;

        for (auto &&[vel, c] : Zipper(velocities, controls))
        {
            vel.vx = c.inputX * speed;
            vel.vy = c.inputY * speed;
            // shooting can be handled separately
        }
    }

    inline void draw_system(Registry &r,
        Sparse_array<Component::position> &positions,
        Sparse_array<Component::drawable> &drawables,
        R_Graphic::Window &window)
    {
        for (auto &&[pos, dr] : Zipper(positions, drawables))
        {
            if (dr.texture) {
                    dr.texture->position = {pos.x, pos.y};
                    dr.texture->draw(window, &dr.rect);
            }
        }
    }
}
