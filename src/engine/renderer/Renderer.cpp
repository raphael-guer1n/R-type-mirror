#include "Renderer.hpp"
#include <SDL.h>

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
