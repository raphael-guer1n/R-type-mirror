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
    SDL_SetRenderDrawColor(_window.getRenderer(), 255, 255, 255, 255);
    SDL_RenderClear(_window.getRenderer());
}

void engine::R_Graphic::Renderer::display() {
    SDL_RenderPresent(_window.getRenderer());
}

void engine::R_Graphic::Renderer::setDrawColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    SDL_SetRenderDrawColor(_window.getRenderer(), r, g, b, a);
}

void engine::R_Graphic::Renderer::fillRect(int x, int y, int w, int h) {
    SDL_Rect rect{ x, y, w, h };
    SDL_RenderFillRect(_window.getRenderer(), &rect);
}

void engine::R_Graphic::Renderer::drawRect(int x, int y, int w, int h) {
    SDL_Rect rect{ x, y, w, h };
    SDL_RenderDrawRect(_window.getRenderer(), &rect);
}
