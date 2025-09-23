#include "R_Graphic/Window.hpp"
#include <iostream>

R_Graphic::Window::Window(const std::string &title, int width, int height)
    : _window(nullptr), _renderer(nullptr), _isOpen(true)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Error SDL_Init: " << SDL_GetError() << std::endl;
        _isOpen = false;
        return;
    }
    _window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!_window) {
        std::cerr << "Error SDL_CreateWindow: " << SDL_GetError() << std::endl;
        _isOpen = false;
        return;
    }
    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!_renderer) {
        std::cerr << "Error SDL_CreateRenderer: " << SDL_GetError() << std::endl;
        _isOpen = false;
    }
}

R_Graphic::Window::~Window() {
    if (_renderer) SDL_DestroyRenderer(_renderer);
    if (_window) SDL_DestroyWindow(_window);
    SDL_Quit();
}

bool R_Graphic::Window::isOpen() const {
    return _isOpen;
}

void R_Graphic::Window::pollEvents(bool &running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            running = false;
    }
}

SDL_Window *R_Graphic::Window::getWindow() const
{
    return _window;
}

SDL_Renderer *R_Graphic::Window::getRenderer() const
{
    return _renderer;
}
