#pragma once
#include <SDL.h>
#include <string>

namespace R_Graphic
{
    class Window {
        public:
            Window(const std::string &title, int width, int height);
            ~Window();
            SDL_Renderer* getRenderer() const;
            bool isOpen() const;
            void pollEvents(bool &running);
            SDL_Window* getWindow() const;
        private:
            SDL_Window* _window;
            SDL_Renderer* _renderer;
            bool _isOpen;
    };
}

