#include <SDL_image.h>
#include <sstream>
#include "Texture.hpp"
#include "Error.hpp"

/**
 * @file Texture.cpp
 * @brief Implementation of the Texture class, responsible for handling and rendering textures using SDL2.
 *
 * The Texture class encapsulates the process of loading, managing, and displaying
 * 2D textures within the R-Type graphical engine. It uses SDL2 for image handling
 * and rendering, providing a convenient abstraction layer for texture operations.
 *
 * @details
 * - Loads image files via SDL_image.
 * - Converts loaded surfaces into SDL textures.
 * - Supports position and size management for each texture instance.
 * - Provides drawing functionality with optional source rectangles.
 * - Automatically releases SDL texture resources upon destruction.
 *
 * @namespace engine::R_Graphic
 * Namespace containing all graphical rendering components of the engine.
 *
 * @class engine::R_Graphic::Texture
 * @brief Represents a 2D texture that can be rendered to a window.
 *
 * @param window Reference to the rendering window used to create the texture.
 * @param filepath Path to the image file to be loaded.
 * @param pos Initial position of the texture on the screen.
 * @param size Desired size of the texture. If (0,0), the image’s original size is used.
 *
 * @throws engine::Error If image loading or texture creation fails.
 *
 * @see engine::R_Graphic::Window
 * @see engine::R_Graphic::Renderer
 */

engine::R_Graphic::Texture::Texture(R_Graphic::Window& window,
    const std::string &filepath, R_Graphic::doubleVec2 pos, R_Graphic::intVec2 size)
: position(pos), _size(size), _texture(nullptr)
{
    std::ostringstream oss;
    SDL_Surface* surface = IMG_Load(filepath.c_str());

    if (!surface) {
        oss << "Texture: Error load img: " << IMG_GetError();
        throw engine::Error(oss.str());
    }

    _texture = SDL_CreateTextureFromSurface(window.getRenderer(), surface);
    if (!_texture) {
        oss << "Texture: Error creating texture: " << SDL_GetError();
        SDL_FreeSurface(surface);
        throw engine::Error(oss.str());
    }

    if (_size.x == 0 || _size.y == 0) {
        _size.x = surface->w;
        _size.y = surface->h;
    }
    SDL_FreeSurface(surface);
}

void engine::R_Graphic::Texture::draw(R_Graphic::Window& window, R_Graphic::textureRect* srcrect) {
    SDL_Rect dst = {
        static_cast<int>(position.x),
        static_cast<int>(position.y),
        _size.x, _size.y
    };

    if (!srcrect) {
        SDL_RenderCopy(window.getRenderer(), _texture, nullptr, &dst);
    } else {
        SDL_Rect rect = {
            srcrect->pos.x, srcrect->pos.y,
            srcrect->size.x, srcrect->size.y
        };
        SDL_RenderCopy(window.getRenderer(), _texture, &rect, &dst);
    }
}

engine::R_Graphic::intVec2 engine::R_Graphic::Texture::getSize() const {
    return _size;
}

void engine::R_Graphic::Texture::setPosition(double x, double y) {
    position.x = x;
    position.y = y;
}

engine::R_Graphic::Texture::~Texture() {
    if (_texture)
        SDL_DestroyTexture(_texture);
}
