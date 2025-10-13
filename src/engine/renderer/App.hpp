#pragma once
#include <string>
#include <chrono>
#include <memory>
#include <functional>
#include "Window.hpp"
#include "Renderer.hpp"
#include "engine/events/Events.hpp"
#include "engine/audio/Music.hpp"

/**
 * @file App.hpp
 * @brief Definition of the App class, responsible for managing the main loop of the R-Type engine.
 *
 * The App class serves as the central component of the graphical engine.
 * It encapsulates a rendering window and a renderer, and provides a simple interface
 * to manage the main application loop, including update and draw phases.
 *
 * @details
 * - Holds references to the rendering window and renderer.
 * - Handles the collection and propagation of user input events.
 * - Manages a Music instance to control background audio playback.
 * - Provides the run() method to execute the game loop, invoking user-defined
 *   update and draw functions each frame.
 *
 * @namespace engine::R_Graphic
 * Namespace grouping all graphical components of the engine.
 *
 * @class engine::R_Graphic::App
 * @brief Core application class handling window, rendering, events, and audio.
 *
 * @see engine::R_Graphic::Window
 * @see engine::R_Graphic::Renderer
 * @see engine::R_Events::Event
 * @see engine::audio::Music
 */

namespace engine {
    namespace R_Graphic
    {
        class App {
            public:
                App(const std::string name, const int width, const int height);
                ~App() = default;
                void run(std::function<void(float,
                    const std::vector<R_Events::Event>&)> gameUpdate,
                    std::function<void()> gameDraw);
                Window& getWindow();
                const std::vector<R_Events::Event>& getEvents() const {
                    return _events;
                }
                Renderer& getRenderer();
                engine::audio::Music& getMusic() { return _music; }
            private:
                std::vector<R_Events::Event> _events;
                std::unique_ptr<Renderer> _renderer;
                Window _window;
                engine::audio::Music _music;
        };
    }
}