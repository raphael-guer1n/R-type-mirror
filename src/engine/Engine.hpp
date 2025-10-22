#pragma once

// Core engine headers (always available)
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Sparse_array.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "engine/ecs/Components.hpp"
#include "engine/ecs/Systems.hpp"

// Conditional subsystem includes
#ifdef ENGINE_HAS_RENDERER
#include "engine/renderer/App.hpp"
#include "engine/renderer/Renderer.hpp"
#include "engine/renderer/Texture.hpp"
#include "engine/renderer/Window.hpp"
#include "engine/renderer/Vectors.hpp"
#include "engine/renderer/Error.hpp"
#endif

#ifdef ENGINE_HAS_AUDIO
#include "engine/audio/Music.hpp"
#endif

#ifdef ENGINE_HAS_NETWORK
#include "engine/network/IoContext.hpp"
#include "engine/network/UdpSocket.hpp"
#include "engine/network/Endpoint.hpp"
#endif

#ifdef ENGINE_HAS_EVENTS
#include "engine/events/Events.hpp"
#endif

namespace Engine {
    // Subsystem availability checks
    constexpr bool HasRenderer() {
#ifdef ENGINE_HAS_RENDERER
        return true;
#else
        return false;
#endif
    }

    constexpr bool HasAudio() {
#ifdef ENGINE_HAS_AUDIO
        return true;
#else
        return false;
#endif
    }

    constexpr bool HasNetwork() {
#ifdef ENGINE_HAS_NETWORK
        return true;
#else
        return false;
#endif
    }

    constexpr bool HasECS() {
#ifdef ENGINE_HAS_ECS
        return true;
#else
        return false;
#endif
    }

    constexpr bool HasEvents() {
#ifdef ENGINE_HAS_EVENTS
        return true;
#else
        return false;
#endif
    }
}