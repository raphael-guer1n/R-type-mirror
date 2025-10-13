#pragma once
#include "engine/renderer/Texture.hpp"

/**
 * @file Components_client_sdl.hpp
 * @brief Defines the drawable component for SDL client rendering.
 *
 * This file contains the definition of the `component::drawable` struct, which is used to represent
 * renderable entities in the SDL client. The drawable component encapsulates the texture, its rectangle
 * (position and size), and the rendering layer for proper ordering.
 */

namespace component
{

    /**
     * @struct drawable
     * @brief Component representing a renderable entity with texture and position information.
     *
     * The `drawable` struct is designed to be attached to entities that need to be rendered on the screen.
     * It holds a shared pointer to a texture, a rectangle specifying the portion of the texture to display,
     * and a layer value to control rendering order.
     *
     * @var rect
     *      The rectangle specifying the position and size of the texture to be rendered.
     * @var texture
     *      Shared pointer to the texture resource to be used for rendering.
     * @var layer
     *      Integer value indicating the rendering layer; higher values are rendered above lower ones.
     *
     * @note
     *      The default constructor initializes the layer to 0 and uses default values for the rectangle.
     *
     * @param tex
     *      Shared pointer to the texture resource.
     * @param r
     *      Rectangle specifying the portion of the texture to render (default is an empty rectangle).
     * @param lay
     *      Rendering layer (default is 0).
     */
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
