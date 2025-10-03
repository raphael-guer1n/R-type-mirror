#pragma once
#include <SDL.h>
#include <string>
#include <vector>
#include "Vectors.hpp"
#include "engine/events/Events.hpp"

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
