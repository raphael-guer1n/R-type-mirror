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
        _registry.register_component<component::drawable>();
        _registry.register_component<component::tion>();
        _registry.register_component<component::velocity>();
        _registry.register_component<component::controllable>();
        _registry.register_component<component::player_tag>();
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
    auto& positions = _registry.get_components<component::position>();
    auto& controls = _registry.get_components<component::controllable>();
    auto& velocities = _registry.get_components<component::velocity>();
    auto& drawables = _registry.get_components<component::drawable>();
    auto& players    = _registry.get_components<component::player_tag>();
    engine::input_system(_registry, controls);
    engine::control_system(_registry, velocities, controls);
    engine::position_system(_registry, positions, velocities, deltaTime);
    engine::boundary_system(_registry, positions, drawables, players, _app.getWindow());
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

engine::registry &R_Type::Rtype::getRegistry()
{
    return _registry;
}

}
