#include <iostream>
#include "Rtype.hpp"
#include "R_Graphic/Vectors.hpp"
#include "R_Graphic/Error.hpp"
#include "R_Ecs/Systems.hpp"
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
        _registry.register_component<R_Ecs::Component::controllable>();
        _registry.register_component<R_Ecs::Component::player_tag>();
        _background = std::make_unique<Background>(*this);
        _player = std::make_unique<Player>(*this);
    }
    catch(const R_Graphic::Error& e)
    {
        throw R_Graphic::Error(e);
    }
}

void R_Type::Rtype::update(float deltaTime)
{
    _background->update(_app, _registry, deltaTime);
    auto& positions = _registry.get_components<R_Ecs::Component::position>();
    auto& controls = _registry.get_components<R_Ecs::Component::controllable>();
    auto& velocities = _registry.get_components<R_Ecs::Component::velocity>();
    auto& drawables = _registry.get_components<R_Ecs::Component::drawable>();
    auto& players    = _registry.get_components<R_Ecs::Component::player_tag>();
    R_Ecs::input_system(_registry, controls);
    R_Ecs::control_system(_registry, velocities, controls);
    R_Ecs::position_system(_registry, positions, velocities, deltaTime);
    R_Ecs::boundary_system(_registry, positions, drawables, players, _app.getWindow());
}

void R_Type::Rtype::draw()
{
    _background->draw(_app, _registry);
    R_Graphic::textureRect player_texture_rect(100, 0, 34, 20);
}

R_Graphic::App& R_Type::Rtype::getApp()
{
    return _app;
}

R_Ecs::Registry &R_Type::Rtype::getRegistry()
{
    return _registry;
}

