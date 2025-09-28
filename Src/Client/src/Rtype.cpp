#include <iostream>
#include "Rtype.hpp"
#include "R_Graphic/Vectors.hpp"
#include "R_Graphic/Error.hpp"
#include "R_Ecs/Components.hpp"

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
            R_Graphic::doubleVec2(0, 0),
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
