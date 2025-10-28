/**
 * @file App.cpp
 * @brief Implementation of the App class for the R-Graphic engine.
 *
 * This file contains the implementation of the App class, which manages the main application loop,
 * window creation, event polling, and rendering for the R-Graphic engine.
 */

/// @class engine::R_Graphic::App
/// @brief Main application class for the R-Graphic engine.
///
/// The App class is responsible for initializing the rendering window, managing the main loop,
/// handling events, and coordinating the update and draw phases of the application.

/**
 * @brief Constructs an App object with the specified window name, width, and height.
 * @param name The title of the window.
 * @param width The width of the window in pixels.
 * @param height The height of the window in pixels.
 * @throws Error if the window fails to open.
 */
 
/**
 * @brief Runs the main application loop.
 * 
 * This function continuously polls events, updates the game state, clears the renderer,
 * draws the game, and displays the rendered frame until the application is closed.
 * 
 * @param gameUpdate Callback function for updating the game logic. Receives delta time and events.
 * @param gameDraw Callback function for drawing the game.
 */
 
/**
 * @brief Returns a reference to the application's window.
 * @return Reference to the Window object managed by the App.
 */
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
