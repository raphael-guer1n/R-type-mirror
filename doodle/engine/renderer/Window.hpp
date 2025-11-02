#pragma once
#include <SDL.h>
#include <string>
#include <vector>
#include "Vectors.hpp"
#include "engine/events/Events.hpp"

/**
 * @file Window.hpp
 * @brief Definition of the Window class for managing SDL windows and rendering contexts.
 *
 * The Window class encapsulates the creation, management, and destruction of an SDL window
 * and its associated renderer. It also provides event polling and access to rendering
 * information such as window size and status.
 *
 * @details
 * - Handles initialization and cleanup of SDL window and renderer.
 * - Provides access to the SDL_Renderer for drawing operations.
 * - Offers event polling through the pollEvents() function, converting SDL events
 *   into the engine’s unified event system.
 * - Allows checking whether the window is open and retrieving its size or handle.
 *
 * @namespace engine::R_Graphic
 * Namespace grouping all rendering-related classes of the engine.
 *
 * @class engine::R_Graphic::Window
 * @brief Encapsulates an SDL window and renderer, handling display and event input.
 *
 * @param title Title of the window displayed on the OS window bar.
 * @param width Width of the window in pixels.
 * @param height Height of the window in pixels.
 *
 * @see engine::R_Graphic::Renderer
 * @see engine::R_Events::Event
 */

namespace engine
{
    namespace R_Graphic
    {
        class Window
        {
            public:
                Window(const std::string &title, int width, int height);
                ~Window();
                SDL_Renderer *getRenderer() const;
                bool isOpen() const;
                std::vector<R_Events::Event> pollEvents(bool &running);
                SDL_Window *getWindow() const;
                intVec2 getSize();

            private:
                SDL_Window *_window;
                SDL_Renderer *_renderer;
                bool _isOpen;
        };
    }
}
