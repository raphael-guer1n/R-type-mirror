#include "Renderer.hpp"
#include <SDL.h>

/**
 * @file Renderer.cpp
 * @brief Implementation of the Renderer class for handling rendering operations using SDL.
 *
 * This file contains the implementation of the Renderer class methods, which provide
 * basic rendering functionalities such as clearing the screen and presenting the rendered
 * content to the window. The Renderer operates on a given Window instance and utilizes
 * SDL's rendering API.
 */
engine::R_Graphic::Renderer::Renderer(Window &window)
: _window(window)
{
}

void engine::R_Graphic::Renderer::clear() {
    SDL_RenderClear(_window.getRenderer());
}

void engine::R_Graphic::Renderer::display() {
    SDL_RenderPresent(_window.getRenderer());
}
