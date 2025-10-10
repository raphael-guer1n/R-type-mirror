#include "Error.hpp"
#include "App.hpp"
#include <iostream>

engine::R_Graphic::App::App(const std::string name, const int width, const int height)
: _window(name, width, height)
{
    if (!_window.isOpen())
        throw Error("R-Graphic: Error on Window");
    _renderer = std::make_unique<Renderer>(_window);
}

void engine::R_Graphic::App::run(std::function<void(float,
    const std::vector<R_Events::Event>&)> gameUpdate,
    std::function<void()> gameDraw)
{
    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;

    while (running) {
        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - lastTime).count();
        auto events = _window.pollEvents(running);

        lastTime = now;
        gameUpdate(deltaTime, events);
        _renderer->clear();
        gameDraw();
        _renderer->display();
    }
}

engine::R_Graphic::Window& engine::R_Graphic::App::getWindow()
{
    return _window;
}

engine::R_Graphic::Renderer& engine::R_Graphic::App::getRenderer() 
{
    return *_renderer;
}