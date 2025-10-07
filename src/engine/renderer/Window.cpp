#include "Window.hpp"
#include <iostream>


engine::R_Graphic::Window::Window(const std::string &title, int width, int height)
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

engine::R_Graphic::Window::~Window() {
    if (_renderer) SDL_DestroyRenderer(_renderer);
    if (_window) SDL_DestroyWindow(_window);
    SDL_Quit();
}

bool engine::R_Graphic::Window::isOpen() const {
    return _isOpen;
}

std::vector<engine::R_Events::Event> engine::R_Graphic::Window::pollEvents(bool &running) {
    std::vector<engine::R_Events::Event> events;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type)
        {
            case SDL_QUIT:
                running = false;
                events.push_back({R_Events::Type::Quit});
                break;
            case SDL_KEYDOWN: {
                engine::R_Events::Event e{};
                e.type = engine::R_Events::Type::KeyDown;
                e.key.code = engine::R_Events::mapSDLKey(event.key.keysym.sym);
                events.push_back(e);
                break;
            }
            case SDL_KEYUP: {
                engine::R_Events::Event e{};
                e.type = engine::R_Events::Type::KeyUp;
                e.key.code = engine::R_Events::mapSDLKey(event.key.keysym.sym);
                events.push_back(e);
                break;
            }
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        events.push_back({R_Events::Type::FocusGained});
                        break;
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        events.push_back({R_Events::Type::FocusLost});
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    return events;
}

SDL_Window *engine::R_Graphic::Window::getWindow() const
{
    return _window;
}

engine::R_Graphic::intVec2 engine::R_Graphic::Window::getSize()
{
    int width, height;

    SDL_GetWindowSize(_window, &width, &height);
    return intVec2(width, height);
}
SDL_Renderer *engine::R_Graphic::Window::getRenderer() const
{
    return _renderer;
}
