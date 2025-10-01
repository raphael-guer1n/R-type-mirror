#pragma once
#include <cstdint>

namespace component
{
    // Tag component to mark the local player's entity on the client
    struct local_player_tag {};

    // Optional tag to mark remote players (other clients)
    struct remote_player_tag {};
}
