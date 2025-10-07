#pragma once
#include <memory>
#include "engine/renderer/Texture.hpp"
#include "engine/renderer/App.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Sparse_array.hpp"
#include "common/Components.hpp"
#include "common/Components_client.hpp"
#include "common/Components_client_sdl.hpp"

namespace R_Type
{
    class Rtype;
    class Background
    {
        public:
            Background(R_Type::Rtype& rtype);
            ~Background() = default;
    };
}
