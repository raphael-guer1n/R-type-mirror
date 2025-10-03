#pragma once
#include <SDL.h>
#include <string>
#include "Vectors.hpp"
#include "Window.hpp"
/**
 * @file Texture.hpp
 * @brief Defines the Texture class for handling image textures in the rendering engine.
 *
 * This header provides the Texture class, which manages loading, positioning, and drawing
 * textures using SDL. It also defines the textureRect struct for specifying subregions of textures.
 */
namespace R_Graphic {
    struct textureRect {
        textureRect() = default;
        textureRect(int x, int y, int w, int h) {
            pos.x = x; pos.y = y;
            size.x = w; size.y = h;
        }
        intVec2 pos;
        intVec2 size;
    };

    class Texture {
    public:
        Texture(Window& window, const std::string& filepath, doubleVec2 pos = {0, 0}, intVec2 size = {0, 0});
        ~Texture();
        void setPosition(double x, double y);
        void draw(Window& window, textureRect* srcrect);
        intVec2 getSize() const;
        doubleVec2 position;
    private:
        SDL_Texture* _texture;
        intVec2 _size;
    };
}
