#pragma once
#include <SDL.h>
#include <string>
#include "Vectors.hpp"
#include "Window.hpp"

namespace R_Graphic
{
    class Texture
    {
        public:
            Texture(Window& window, const std::string& filepath,
                doubleVec2 pos = {0, 0}, intVec2 size = {0, 0});
            ~Texture();
            void draw(R_Graphic::Window& window, SDL_Rect* srcrect);
        public:
            doubleVec2 position;
        private:
            SDL_Texture *_texture;
            intVec2 _size;
    };
}
