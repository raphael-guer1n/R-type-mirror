#pragma once
#include <memory>
#include "engine/renderer/Texture.hpp"
#include "engine/renderer/App.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Sparse_array.hpp"
#include "common/Components.hpp"
#include "common/Components_client.hpp"
#include "common/Components_client_sdl.hpp"

/**
 * @file Background.hpp
 * @brief Defines the Background class for the R-Type client.
 *
 * This header declares the Background class, which is responsible for managing
 * and rendering the background elements in the R-Type game client. It interacts
 * with the main Rtype game class and utilizes various engine components such as
 * textures, application context, and ECS registry to handle background logic.
 *
 * @author marysekatary
 * @date 2024
 */

namespace R_Type
    /**
     * @class Background
     * @brief Handles the background rendering and logic for the R-Type client.
     *
     * The Background class encapsulates all functionalities related to the
     * background visuals in the R-Type game client. It maintains a reference to
     * the main Rtype game instance and leverages engine and common components
     * for efficient background management.
     *
     * @note This class is intended to be used internally by the R-Type client.
     */

{
    class Rtype;
    class Background
    {
        public:
            Background(R_Type::Rtype& rtype);
            ~Background() = default;
    };
}
