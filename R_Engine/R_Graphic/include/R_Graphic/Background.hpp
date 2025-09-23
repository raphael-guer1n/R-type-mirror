#pragma once
#include <SDL.h>
#include <string>

namespace R_Graphic
{
    class Background
    {
    public:
        Background(SDL_Renderer *renderer, SDL_Window *window, const std::string &filepath, float speed);
        ~Background();
        void update(float deltaTime);
        void draw(SDL_Renderer *renderer);

    private:
        SDL_Texture *_texture;
        float _x1;
        float _x2;
        float _speed;
        int _winWidth;
        int _winHeight;
    };
}

