#pragma once
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

namespace component
{
    struct drawable
    {
        sf::Vector2f size{30.f, 30.f};
        sf::Color color{sf::Color::White};
        float renderX, renderY;
        drawable() = default;
        drawable(sf::Vector2f s, sf::Color c) : size(s), color(c) {}
    };
}
