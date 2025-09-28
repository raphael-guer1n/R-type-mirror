#pragma once
#include <SDL.h>
#include <string>
#include "Vectors.hpp"
#include "Window.hpp"

namespace R_Graphic
{
    struct textureRect {
        textureRect() = default;
        textureRect(int x, int y, int w, int h) {pos.x = x; pos.y = y;
            size.x = w; size.y = h;}
        intVec2 pos;
        intVec2 size;
    };
    class Texture
    {
        public:
            Texture(Window& window, const std::string& filepath,
                doubleVec2 pos = {0, 0}, intVec2 size = {10, 10});
            ~Texture();
            void draw(R_Graphic::Window& window, textureRect* srcrect);
        public:
            doubleVec2 position;
        private:
            SDL_Texture *_texture;
            intVec2 _size;
    };
}
