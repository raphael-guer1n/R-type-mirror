/*
** EPITECH PROJECT, 2025
** rtype-mirror
** File description:
** Main.cpp
*/
#include "R_Graphic/Window.hpp"
#include "R_Graphic/Renderer.hpp"

int main() {
    Window window("R-Type", 800, 600);
    bool running = true;
    if (!window.isOpen())
        return 1;
    Renderer renderer(window);
    while (running) {
        window.pollEvents(running);
        renderer.clear();
        renderer.display();
    }
    return 0;
}
