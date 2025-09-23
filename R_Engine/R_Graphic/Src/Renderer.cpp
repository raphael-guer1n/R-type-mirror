#include "R_Graphic/Renderer.hpp"
#include <SDL.h>

Renderer::Renderer(Window &window)
: _window(window)
{
}

void Renderer::clear() {
    SDL_RenderClear(_window.getRenderer());
}

void Renderer::display() {
    SDL_RenderPresent(_window.getRenderer());
}
