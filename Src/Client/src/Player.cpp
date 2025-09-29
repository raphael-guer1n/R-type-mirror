#include <iostream>
#include "R_Ecs/Systems.hpp"
#include "Rtype.hpp"
#include "Player.hpp"

R_Type::Player::Player(R_Type::Rtype &rtype)
{
    auto& registry = rtype.getRegistry();
    auto e = registry.spawn_entity();

    R_Graphic::textureRect rect(100, 0, 34, 20);
    int winH = rtype.getApp().getWindow().getSize().y;
    registry.add_component(e, R_Ecs::Component::position{50.0f, winH / 2.0f});
    registry.add_component(e, R_Ecs::Component::velocity{-100.0f, 0.0f});
    registry.add_component(e, R_Ecs::Component::controllable{});
    registry.add_component(e, R_Ecs::Component::player_tag{});
    auto player = std::make_shared<R_Graphic::Texture>(
            rtype.getApp().getWindow(),
            "../../Assets/sprites/r-typesheet1.gif",
            R_Graphic::doubleVec2(100, 300),  
            R_Graphic::intVec2(132, 72)       
        );
    registry.emplace_component<R_Ecs::Component::drawable>(e, player, rect);
}
