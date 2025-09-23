/*
** EPITECH PROJECT, 2025
** rtype-mirror
** File description:
** Main.cpp
*/
#include <chrono>
#include "Rtype.hpp"

int main() {
    R_Type::Rtype game;

    game.getApp().run(
        [&game](float dt) { game.update(dt); },
        [&game]() { game.draw(); }
    );
    return 0;
}
