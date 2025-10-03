#pragma once
#include <SFML/Graphics.hpp>
#include "engine/ecs/Registry.hpp"
#include "common/Components.hpp"
#include "common/Components_client_sfml.hpp"
#include "engine/ecs/iterator/Zipper.hpp"
#include "engine/ecs/iterator/Indexed_zipper.hpp"
#include "engine/renderer/Window.hpp"

using namespace engine;

inline void control_system(registry &r,
                    sparse_array<component::velocity> &velocities,
                    sparse_array<component::controllable> &controls)
{
    for (auto &&[i, vel, c] : indexed_zipper(velocities, controls))
    {
        vel.vx = 0.f;
        vel.vy = 0.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            vel.vx = -14.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            vel.vx = 14.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            vel.vy = -14.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            vel.vy = 14.f;
    }
}


inline void draw_system(registry &r,
    sparse_array<component::position> &positions,
    sparse_array<component::drawable> &drawables,
    R_Graphic::Window &window)
{
    for (auto &&[pos, dr] : zipper(positions, drawables))
    {
        if (dr.texture) {
                dr.texture->position = {pos.x, pos.y};
                dr.texture->draw(window, &dr.rect);
        }
    }
}