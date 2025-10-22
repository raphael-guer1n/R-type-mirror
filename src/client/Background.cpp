/**
 * @brief Initializes the background entities for the R-Type client.
 *
 * This constructor creates two background entities to enable a seamless scrolling starfield effect.
 * Each entity is assigned a position, velocity, and decor kind, and is associated with a drawable
 * component using the starfield texture. The second entity is positioned to the right of the first,
 * allowing for continuous background movement as the game progresses.
 *
 * @param rtype Reference to the main Rtype application, used to access the window, registry, and resources.
 */
#include <iostream>
#include "Background.hpp"
#include "common/Layers.hpp"
#include "engine/ecs/Systems.hpp"
#include "Rtype.hpp"

R_Type::Background::Background(R_Type::Rtype& rtype)
{
    R_Graphic::intVec2 pos = rtype.getApp().getWindow().getSize();
    auto& registry = rtype.getRegistry();
    auto e = registry.spawn_entity();

    registry.add_component(e, component::position{0.0f, 0.0f});
    registry.add_component(e, component::velocity{-100.0f, 0.0f});
    registry.add_component(e, component::entity_kind{component::entity_kind::decor});
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
    registry.emplace_component<component::drawable>(e, tex, rect, layers::Background);
    auto e1 = registry.spawn_entity();
    registry.add_component(e1, component::position{static_cast<float>(pos.x), 0.0f});
    registry.add_component(e1, component::velocity{-100.0f, 0.0f});
    registry.add_component(e1, component::entity_kind{component::entity_kind::decor});
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
    registry.emplace_component<component::drawable>(e1, tex1, rect1, layers::Background);
}
