/*
** EPITECH PROJECT, 2025
** rtype-mirror
** File description:
** Main.cpp
*/
#include "R_Graphic/Window.hpp"
#include "R_Graphic/Renderer.hpp"
#include "R_Graphic/Background.hpp"
#include <chrono>

int main() {
    Window window("R-Type", 800, 600);
    Background background(window.getRenderer(), window.getWindow(), "../../Assets/Background/Starfield.png", 100.0f);
    auto lastTime = std::chrono::high_resolution_clock::now();

    bool running = true;
    if (!window.isOpen())
        return 1;
    Renderer renderer(window);
    while (running) {
        window.pollEvents(running);
        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        background.update(deltaTime);
        renderer.clear();
        background.draw(window.getRenderer());
        renderer.display();
    }
    return 0;
}
