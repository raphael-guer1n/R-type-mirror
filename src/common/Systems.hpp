#pragma once

#include <SFML/Graphics.hpp>
#include "engine/ecs/Registry.hpp"
#include "common/Components.hpp"
#include "engine/ecs/iterator/Zipper.hpp"
#include "engine/ecs/iterator/Indexed_zipper.hpp"

void position_system(engine::registry &r, engine::sparse_array<component::position> &positions,
    engine::sparse_array<component::velocity> &velocities)
{
    for (auto &&[i, pos, vel] : engine::indexed_zipper(positions, velocities))
    {
        pos.x += vel.vx;
        pos.y += vel.vy;
    }
}

void control_system(engine::registry &r,
    engine::sparse_array<component::velocity> &velocities,
    engine::sparse_array<component::controllable> &controls)
{
    constexpr float speed = 5.f;

    for (auto &&[i, vel, c] : engine::indexed_zipper(velocities, controls))
    {
        vel.vx = c.inputX * speed;
        vel.vy = c.inputY * speed;
        // shooting can be handled separately
    }
}

void draw_system(engine::registry &r, engine::sparse_array<component::position> &positions,
    engine::sparse_array<component::drawable> &drawables, sf::RenderWindow &window)
{
    for (auto &&[i, pos, dr] : engine::indexed_zipper(positions, drawables))
    {
        sf::RectangleShape shape(dr.size);
        shape.setFillColor(dr.color);
        shape.setPosition(pos.x, pos.y);
        window.draw(shape);
    }
}
