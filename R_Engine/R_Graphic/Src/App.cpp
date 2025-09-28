#include "R_Graphic/Error.hpp"
#include "R_Graphic/App.hpp"
#include <iostream>

R_Graphic::App::App(const std::string name, const int width, const int height)
: _window(name, width, height)
{
    if (!_window.isOpen())
        throw Error("R-Graphic: Error on Window");
    _renderer = std::make_unique<Renderer>(_window);
}

void R_Graphic::App::run(std::function<void(float)> gameUpdate,
    std::function<void()> gameDraw)
{
    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;

    while (running) {
        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        _window.pollEvents(running);
        gameUpdate(deltaTime);
        _renderer->clear();
        gameDraw();
        _renderer->display();
    }
}

R_Graphic::Window& R_Graphic::App::getWindow()
{
    return _window;
}
