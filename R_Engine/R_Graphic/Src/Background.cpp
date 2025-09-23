#include "R_Graphic/Background.hpp"
#include <SDL_image.h>
#include <iostream>

R_Graphic::Background::Background(SDL_Renderer* renderer, SDL_Window* window, const std::string& filepath, float speed)
    : _texture(nullptr), _x1(0), _x2(0), _speed(speed)
{
    SDL_Surface* surface = IMG_Load(filepath.c_str());
    if (!surface) {
        std::cerr << "Error load img: " << IMG_GetError() << std::endl;
        return;
    }
    _texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!_texture) {
        std::cerr << "Error create texture: " << SDL_GetError() << std::endl;
        return;
    }
    SDL_GetWindowSize(window, &_winWidth, &_winHeight);
    _x1 = 0.0f;
    _x2 = static_cast<float>(_winWidth);
}

R_Graphic::Background::~Background() {
    if (_texture) SDL_DestroyTexture(_texture);
}

void R_Graphic::Background::update(float deltaTime) {
    float move = _speed * deltaTime;

    _x1 -= move;
    _x2 -= move;
    if (_x1 + _winWidth <= 0) _x1 = _x2 + _winWidth;
    if (_x2 + _winWidth <= 0) _x2 = _x1 + _winWidth;
}

void R_Graphic::Background::draw(SDL_Renderer* renderer) {
    SDL_Rect dst1 = {static_cast<int>(_x1), 0, _winWidth, _winHeight};
    SDL_Rect dst2 = {static_cast<int>(_x2), 0, _winWidth, _winHeight};
    SDL_RenderCopy(renderer, _texture, nullptr, &dst1);
    SDL_RenderCopy(renderer, _texture, nullptr, &dst2);
}
