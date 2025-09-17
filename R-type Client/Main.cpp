#include <SFML/Graphics.hpp>
#include <ecs/Registry.hpp>
#include <ecs/Components.hpp>
#include <ecs/iterator/Zipper.hpp>
#include <ecs/iterator/Indexed_zipper.hpp>

using namespace engine;

void position_system(registry &r,
                     sparse_array<component::position> &positions,
                     sparse_array<component::velocity> &velocities)
{
    for (auto &&[i, pos, vel] : indexed_zipper(positions, velocities))
    {
        pos.x += vel.vx;
        pos.y += vel.vy;
    }
}

void control_system(registry &r,
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

void draw_system(registry &r,
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

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "ECS Test");
    window.setFramerateLimit(60);
    registry r;
    r.register_component<component::position>();
    r.register_component<component::velocity>();
    r.register_component<component::drawable>();
    r.register_component<component::controllable>();
    auto player = r.spawn_entity();
    r.emplace_component<component::position>(player, 100.f, 100.f);
    r.emplace_component<component::velocity>(player, 0.f, 0.f);
    r.emplace_component<component::drawable>(player, sf::Vector2f{30.f, 30.f},
                                             sf::Color::Green);
    r.emplace_component<component::controllable>(player);
    for (int i = 0; i < 5; i++)
    {
        auto wall = r.spawn_entity();
        r.emplace_component<component::position>(wall, 200.f + i * 50.f, 300.f);
        r.emplace_component<component::drawable>(wall, sf::Vector2f{40.f, 40.f},
                                                 sf::Color::Red);
    }
    r.add_system<component::position, component::velocity>(position_system);
    r.add_system<component::velocity, component::controllable>(control_system);
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        r.run_systems();
        window.clear();
        draw_system(r,
                    r.get_components<component::position>(),
                    r.get_components<component::drawable>(),
                    window);
        window.display();
    }
}
