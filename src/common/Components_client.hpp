#pragma once
#include <cstdint>
/**
 * @file Components_client.hpp
 * @brief Defines ECS components specific to the client side, such as tags for identifying local and remote player entities.
 */
namespace component
{
    // Tag component to mark the local player's entity on the client
    struct local_player_tag {};

    // Optional tag to mark remote players (other clients)
    struct remote_player_tag {};
    
    struct hud_tag {};

    // struct health {
    //     int current;
    //     int max;
    // };

    struct beam_charge {
        float value;
        bool isCharging;
    };

    struct score {
        int current;
        int highScore;
    };
}
