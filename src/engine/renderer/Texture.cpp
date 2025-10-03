#include <SDL_image.h>
#include <sstream>
#include "Texture.hpp"
#include "Error.hpp"

engine::R_Graphic::Texture::Texture(R_Graphic::Window& window,
    const std::string &filepath, R_Graphic::doubleVec2 pos, R_Graphic::intVec2 size)
: position(pos), _size(size), _texture(nullptr)
{
    std::ostringstream oss;
    SDL_Surface* surface = IMG_Load(filepath.c_str());

    if (!surface) {
        oss << "Texture: Error load img: " << IMG_GetError();
        throw R_Graphic::Error(oss.str());
    }

    _texture = SDL_CreateTextureFromSurface(window.getRenderer(), surface);
    if (!_texture) {
        oss << "Texture: Error creating texture: " << SDL_GetError();
        SDL_FreeSurface(surface);
        throw R_Graphic::Error(oss.str());
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
