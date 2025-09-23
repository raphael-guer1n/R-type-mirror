#include "R_Graphic/Renderer.hpp"
#include <SDL.h>

R_Graphic::Renderer::Renderer(Window &window)
: _window(window)
{
}

void R_Graphic::Renderer::clear() {
    SDL_RenderClear(_window.getRenderer());
}

void R_Graphic::Renderer::display() {
    SDL_RenderPresent(_window.getRenderer());
}
