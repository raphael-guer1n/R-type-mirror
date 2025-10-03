#include <iostream>
#include "Background.hpp"
#include "common/Systems.hpp"
#include "Rtype.hpp"

R_Type::Background::Background(R_Type::Rtype& rtype)
{
    R_Graphic::intVec2 pos = rtype.getApp().getWindow().getSize();
    auto& registry = rtype.getRegistry();
    auto e = registry.spawn_entity();

    registry.add_component(e, component::position{0.0f, 0.0f});
    registry.add_component(e, component::velocity{-100.0f, 0.0f});
    registry.add_component(e, component::background_tag{});
    auto tex = std::make_shared<R_Graphic::Texture>(
        rtype.getApp().getWindow(),
        "./Assets/Background/Starfield.png",
        R_Graphic::doubleVec2(0.0, 0.0),
        rtype.getApp().getWindow().getSize()
    );
    R_Graphic::textureRect rect(0, 0,
        rtype.getApp().getWindow().getSize().x,
        rtype.getApp().getWindow().getSize().y
    );
    registry.emplace_component<component::drawable>(e, tex, rect);
    
    auto e1 = registry.spawn_entity();
    registry.add_component(e1, component::position{static_cast<float>(pos.x), 0.0f});
    registry.add_component(e1, component::velocity{-100.0f, 0.0f});
    registry.add_component(e1, component::background_tag{});
    auto tex1 = std::make_shared<R_Graphic::Texture>(
        rtype.getApp().getWindow(),
        "./Assets/Background/Starfield.png",
        R_Graphic::doubleVec2(0.0, 0.0),
        rtype.getApp().getWindow().getSize()
    );
    R_Graphic::textureRect rect1(0, 0,
        rtype.getApp().getWindow().getSize().x,
        rtype.getApp().getWindow().getSize().y
    );
    registry.emplace_component<component::drawable>(e1, tex1, rect1);
    
    std::cout << "Entity e=" << e 
          << " pos=" << registry.get_components<component::position>()[e].has_value()
          << " bg="  << registry.get_components<component::background_tag>()[e].has_value()
          << std::endl;
std::cout << "Entity e1=" << e1 
          << " pos=" << registry.get_components<component::position>()[e1].has_value()
          << " bg="  << registry.get_components<component::background_tag>()[e1].has_value()
          << std::endl;
}

void R_Type::scroll_reset_system(engine::registry& r,
    engine::sparse_array<component::position> &positions,
    engine::sparse_array<component::background_tag> &backgrounds,
    R_Graphic::App &app)
{
    int width = app.getWindow().getSize().x;
    std::vector<component::position*> bg_positions;

    for (size_t i = 0; i < backgrounds.size(); ++i) {
        if (backgrounds[i].has_value() && positions[i].has_value()) {
            bg_positions.push_back(&positions[i].value());
        }
    }
    if (bg_positions.size() < 2) {
    std::cerr << "Error: not enough background entities (" 
              << bg_positions.size() << " found)\n";
    return;
    }
    auto& bg1 = *bg_positions[0];
    auto& bg2 = *bg_positions[1];
    if (bg1.x + width <= 0)
        bg1.x = bg2.x + width;
    if (bg2.x + width <= 0)
        bg2.x = bg1.x + width;
}