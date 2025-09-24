#pragma once
#include <SFML/Graphics.hpp>
#include "Registry.hpp"
#include "Components.hpp"
#include "Components_client_sfml.hpp"
#include "iterator/Zipper.hpp"
#include "iterator/Indexed_zipper.hpp"

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
            vel.vx = -5.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            vel.vx = 5.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            vel.vy = -5.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            vel.vy = 5.f;
    }
}

inline void draw_system(registry &r,
                 sparse_array<component::position> &positions,
                 sparse_array<component::drawable> &drawables,
                 sf::RenderWindow &window)
{
    for (auto &&[i, pos, dr] : indexed_zipper(positions, drawables))
    {
        sf::RectangleShape shape(dr.size);
        shape.setFillColor(dr.color);
        shape.setPosition(pos.x, pos.y);
        window.draw(shape);
    }
}
