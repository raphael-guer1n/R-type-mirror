#pragma once
#include <memory>
#include "engine/renderer/Texture.hpp"
#include "engine/renderer/App.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Sparse_array.hpp"
#include "common/Components.hpp"
#include "common/Components_client.hpp"
#include "common/Components_client_sfml.hpp"

namespace component
{
    struct background_tag {};
}
namespace R_Type
{
    class Rtype;
    void scroll_reset_system(engine::registry& r,
    engine::sparse_array<component::position> &positions,
    engine::sparse_array<component::background_tag> &backgrounds,
    engine::R_Graphic::App &app);
    class Background
    {
        public:
            Background(R_Type::Rtype& rtype);
            ~Background() = default;
    };
}
