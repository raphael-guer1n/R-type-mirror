#pragma once

// Core engine headers (always available)
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Sparse_array.hpp"
#include "engine/ecs/EntityFactory.hpp"
#include "engine/ecs/Components.hpp"
#include "engine/ecs/Systems.hpp"

// Subsystem includes
#include "engine/renderer/App.hpp"
#include "engine/renderer/Renderer.hpp"
#include "engine/renderer/Texture.hpp"
#include "engine/renderer/Window.hpp"
#include "engine/renderer/Vectors.hpp"
#include "engine/renderer/Error.hpp"
#include "engine/audio/Music.hpp"
#include "engine/network/IoContext.hpp"
#include "engine/network/UdpSocket.hpp"
#include "engine/network/Endpoint.hpp"
#include "engine/events/Events.hpp"
#include "engine/profiling/Profiler.hpp"
#include "engine/profiling/ProfilerOverlay.hpp"

namespace Engine {
    // Subsystem availability checks
    constexpr bool HasRenderer() { return true; }
    constexpr bool HasAudio() { return true; }
    constexpr bool HasNetwork() { return true; }
    constexpr bool HasECS() { return true; }
    constexpr bool HasEvents() { return true; }
    constexpr bool HasProfiling() { return true; }
}