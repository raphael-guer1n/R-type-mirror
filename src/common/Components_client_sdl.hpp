#pragma once
#include "engine/renderer/Texture.hpp"

namespace component
{
    struct drawable
    {
        engine::R_Graphic::textureRect rect;
        std::shared_ptr<engine::R_Graphic::Texture> texture;
        int layer = 0;

        drawable() = default;
        drawable(std::shared_ptr<engine::R_Graphic::Texture> tex,
            engine::R_Graphic::textureRect r = engine::R_Graphic::textureRect(),
            int lay = 0) : rect(r), texture(std::move(tex)), layer(lay) {}
    };
}
