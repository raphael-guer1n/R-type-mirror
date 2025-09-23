#pragma once
#include <SDL.h>
#include <string>

class Window {
    public:
        Window(const std::string &title, int width, int height);
        ~Window();
        SDL_Renderer* getRenderer() const;
        bool isOpen() const;
        void pollEvents(bool &running);

    private:
        SDL_Window* _window;
        SDL_Renderer* _renderer;
        bool _isOpen;
};
