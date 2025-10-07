#include "Hud.hpp"
#include "common/Components_client_sdl.hpp"
#include "Rtype.hpp"
#include <memory>

R_Type::Hud::Hud(R_Type::Rtype &rtype)
{
    auto &registry = rtype.getRegistry();
    auto &window = rtype.getApp().getWindow();

    registry.register_component<component::beam_charge>();
    registry.register_component<component::hud_tag>();

    auto e = registry.spawn_entity();

    engine::R_Graphic::doubleVec2 hudPos(500.0, 680.0);

    registry.add_component(e, component::position{
                                  static_cast<float>(hudPos.x),
                                  static_cast<float>(hudPos.y)});
    registry.add_component(e, component::beam_charge{0.0f, false});
    registry.add_component(e, component::hud_tag{});

    auto tex = std::make_shared<engine::R_Graphic::Texture>(
        window,
        "./Assets/Hud/beam_frame.png",
        hudPos,
        engine::R_Graphic::intVec2(500, 120));
    engine::R_Graphic::textureRect rect(0, 530, 1200, 140);
    registry.emplace_component<component::drawable>(e, tex, rect);
}
