/*
** EPITECH PROJECT, 2025
** rtype-mirror
** File description:
** Main.cpp
*/
#include <iostream>
#include "Rtype.hpp"
#include "R_Graphic/Error.hpp"

int main() {
    try
    {
        R_Type::Rtype game;
        game.getApp().run(
            [&game](float dt) { game.update(dt); },
            [&game]() { game.draw(); }
        );
    } catch(const R_Graphic::Error& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
