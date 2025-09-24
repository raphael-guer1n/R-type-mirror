#include <SDL_image.h>
#include <sstream>
#include "R_Graphic/Texture.hpp"
#include "R_Graphic/Error.hpp"

R_Graphic::Texture::Texture(R_Graphic::Window& window,
    const std::string &filepath, doubleVec2 pos, intVec2 size)
{
    std::ostringstream oss;
    SDL_Surface* surface = IMG_Load(filepath.c_str());

    if (!surface) {
        oss << "Texture: Error load img: " << IMG_GetError() << std::endl;
        throw Error(oss.str());
    }
    _texture = SDL_CreateTextureFromSurface(window.getRenderer(), surface);
    SDL_FreeSurface(surface);
    if (!_texture) {
        oss << "Texture: Error creating texture: " << SDL_GetError() << std::endl;
        throw Error(oss.str());
    }
    position = pos;
    _size = size;
}

void R_Graphic::Texture::draw(R_Graphic::Window& window, SDL_Rect* srcrect)
{
    SDL_Rect dst = {static_cast<int>(position.x), 0, _size.x, _size.y};
    if (!srcrect)
        SDL_RenderCopy(window.getRenderer(), _texture, nullptr, &dst);
    else
        SDL_RenderCopy(window.getRenderer(), _texture, srcrect, &dst);
}

R_Graphic::Texture::~Texture()
{
    if (_texture) SDL_DestroyTexture(_texture);
}
