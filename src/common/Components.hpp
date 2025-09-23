#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

namespace component
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
        sf::Vector2f size{30.f, 30.f};
        sf::Color color{sf::Color::White};

        drawable() = default;
        drawable(sf::Vector2f s, sf::Color c) : size(s), color(c) {}
    };
    // Marqueur de contrÃ´le clavier
    struct controllable {
        int inputX = 0; // -1 = left, +1 = right
        int inputY = 0; // -1 = up, +1 = down
        bool shoot = false;
    };

} // namespace component