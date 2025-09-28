#include <iostream>
#include "Rtype.hpp"
#include "R_Graphic/Vectors.hpp"
#include "R_Graphic/Error.hpp"
#include "R_Ecs/Components.hpp"
#include <SDL.h>

R_Type::Rtype::Rtype()
: _app("R-Type", 800, 600)
{
    try
    {
        _registry.register_component<R_Ecs::Component::drawable>();
        _registry.register_component<R_Ecs::Component::position>();
        _registry.register_component<R_Ecs::Component::velocity>();
        _background = std::make_unique<Background>(*this);
        _player = std::make_unique<R_Graphic::Texture>(
            _app.getWindow(),
            "../../Assets/sprites/r-typesheet1.gif",
            R_Graphic::doubleVec2(100, 300),  
            R_Graphic::intVec2(132, 72)       
        );
    }
    catch(const R_Graphic::Error& e)
    {
        throw R_Graphic::Error(e);
    }
}

void R_Type::Rtype::update(float deltaTime)
{
    _background->update(_app, _registry, deltaTime);
    handleInput(deltaTime);
    const Uint8* state = SDL_GetKeyboardState(nullptr);
    auto pos = _player->position;
    float speed = 200.0f;

    if (state[SDL_SCANCODE_UP])    pos.y -= speed * deltaTime;
    if (state[SDL_SCANCODE_DOWN])  pos.y += speed * deltaTime;
    if (state[SDL_SCANCODE_LEFT])  pos.x -= speed * deltaTime;
    if (state[SDL_SCANCODE_RIGHT]) pos.x += speed * deltaTime;
    _player->setPosition(pos.x, pos.y);
}

void R_Type::Rtype::draw()
{
    _background->draw(_app, _registry);
    R_Graphic::textureRect player_texture_rect(100, 0, 34, 20);
    _player->draw(_app.getWindow(), &player_texture_rect);
}

R_Graphic::App& R_Type::Rtype::getApp()
{
    return _app;
}

R_Ecs::Registry &R_Type::Rtype::getRegistry()
{
    return _registry;
}

void R_Type::Rtype::handleInput(float dt)
{
    const Uint8* state = SDL_GetKeyboardState(nullptr);
    auto pos = _player->position;
    float speed = 200.0f;
    int winW = _app.getWindow().getSize().x;
    int winH = _app.getWindow().getSize().y;
    int spriteW = _player->getSize().x;
    int spriteH = _player->getSize().y;

    if (state[SDL_SCANCODE_UP])    
        pos.y -= speed * dt;
    if (state[SDL_SCANCODE_DOWN])
        pos.y += speed * dt;
    if (state[SDL_SCANCODE_LEFT])
        pos.x -= speed * dt;
    if (state[SDL_SCANCODE_RIGHT])
        pos.x += speed * dt;
    if (pos.x < 0)
        pos.x = 0;
    if (pos.x + spriteW > winW)
        pos.x = winW - spriteW;
    if (pos.y < 0) pos.y = 0;
    if (pos.y + spriteH > winH)
        pos.y = winH - spriteH;
    _player->setPosition(pos.x, pos.y);
}
