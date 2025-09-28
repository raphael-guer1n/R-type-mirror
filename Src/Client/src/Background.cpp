#include <iostream>
#include "Background.hpp"
#include "R_Ecs/Systems.hpp"
#include "Rtype.hpp"

R_Type::Background::Background(R_Type::Rtype& rtype)
{
    R_Graphic::intVec2 pos = rtype.getApp().getWindow().getSize();
    auto& registry = rtype.getRegistry();
    registry.register_component<R_Ecs::Component::background_tag>();
    auto e = registry.spawn_entity();

    registry.add_component(e, R_Ecs::Component::position{0.0f, 0.0f});
    registry.add_component(e, R_Ecs::Component::velocity{-100.0f, 0.0f});
    registry.add_component(e, R_Ecs::Component::background_tag{});
    auto tex = std::make_shared<R_Graphic::Texture>(
        rtype.getApp().getWindow(),
        "../../Assets/Background/Starfield.png",
        R_Graphic::doubleVec2(0.0, 0.0),
        rtype.getApp().getWindow().getSize()
    );
    registry.emplace_component<R_Ecs::Component::drawable>(e, tex);
    auto e1 = registry.spawn_entity();
    registry.add_component(e1, R_Ecs::Component::position{static_cast<float>(pos.x), 0.0f});
    registry.add_component(e1, R_Ecs::Component::velocity{-100.0f, 0.0f});
    registry.add_component(e1, R_Ecs::Component::background_tag{});
    auto tex1 = std::make_shared<R_Graphic::Texture>(
        rtype.getApp().getWindow(),
        "../../Assets/Background/Starfield.png",
        R_Graphic::doubleVec2(0.0, 0.0),
        rtype.getApp().getWindow().getSize()
    );
    registry.emplace_component<R_Ecs::Component::drawable>(e1, tex1);
}

void R_Type::Background::update(R_Graphic::App& app, R_Ecs::Registry& reg, float deltaTime)
{
    auto& positions = reg.get_components<R_Ecs::Component::position>();
    auto& velocities = reg.get_components<R_Ecs::Component::velocity>();
    auto& background_tags = reg.get_components<R_Ecs::Component::background_tag>();

    R_Ecs::position_system(reg, positions, velocities, deltaTime);
    scroll_reset_system(reg, positions, background_tags, app);
}

void R_Type::Background::draw(R_Graphic::App &app, R_Ecs::Registry& reg)
{
    auto& positions = reg.get_components<R_Ecs::Component::position>();
    auto& drawables = reg.get_components<R_Ecs::Component::drawable>();

    R_Ecs::draw_system(reg, positions, drawables, app.getWindow());
}

void R_Type::scroll_reset_system(R_Ecs::Registry& r,
    R_Ecs::Sparse_array<R_Ecs::Component::position> &positions,
    R_Ecs::Sparse_array<R_Ecs::Component::background_tag> &backgrounds,
    R_Graphic::App &app)
{
    int width = app.getWindow().getSize().x;
    std::vector<R_Ecs::Component::position*> bg_positions;

    for (size_t i = 0; i < backgrounds.size(); ++i) {
        if (backgrounds[i].has_value() && positions[i].has_value()) {
            bg_positions.push_back(&positions[i].value());
        }
    }
    auto& bg1 = *bg_positions[0];
    auto& bg2 = *bg_positions[1];
    if (bg1.x + width <= 0)
        bg1.x = bg2.x + width;
    if (bg2.x + width <= 0)
        bg2.x = bg1.x + width;
}