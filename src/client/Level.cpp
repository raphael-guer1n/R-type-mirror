/**
 * @file Level.cpp
 * @brief Handles client-side level state transitions and animations for R-Type.
 *
 * This file implements `R_Type::Rtype` class functions responsible for processing server messages
 * that indicate the start or end of a game level. The client maintains a `GameState` to track 
 * whether it is currently playing or loading a level (`_state`), a fade effect alpha value 
 * (`_fadeAlpha`) for transitions, and manages HUD and background updates through `_hud` and 
 * `_background`. When a `LevelStartPayload` is received, the client transitions from LOADING 
 * to PLAYING, triggers a fade-in effect, and instructs the HUD to play the level-start animation.
 * Conversely, when a `LevelEndPayload` is received, the client transitions back to LOADING, 
 * resets the fade effect, updates the background theme for the next level, and logs relevant 
 * state changes. All payloads are safely copied from raw network data using `memcpy` into 
 * structured types for easier access.
 *
 * ### Methods
 * - `handleLevelStart(const std::vector<uint8_t> &payload)` – Handles the server message indicating 
 *   a new level has started, updates game state to PLAYING, triggers HUD animations, and sets 
 *   fade alpha for visual effect.
 * - `handleLevelEnd(const std::vector<uint8_t> &payload)` – Handles the server message indicating 
 *   a level has ended, updates game state to LOADING, resets fade alpha, updates background theme, 
 *   and prepares the client for the next level.
 *
 * @note Payloads from the server are expected to match `LevelStartPayload` and `LevelEndPayload` 
 * structures exactly. Logging via `std::cout` provides runtime feedback for debugging state transitions.
 */

#include <iostream>
#include "Rtype.hpp"

void R_Type::Rtype::handleLevelStart(const std::vector<uint8_t> &payload)
{
    LevelStartPayload p{};
    memcpy(&p, payload.data(), sizeof(LevelStartPayload));
    _state = GameState::PLAYING;
    _fadeAlpha = 255.0f;
    std::cout << "[CLIENT] Leaving LOADING state" << std::endl;
    std::cout << "[CLIENT] LEVEL_START : " << p.level << std::endl;

    _hud->startLevelAnimation(p.level, _registry);
}

void R_Type::Rtype::handleLevelEnd(const std::vector<uint8_t> &payload)
{
    LevelEndPayload p{};
    memcpy(&p, payload.data(), sizeof(LevelEndPayload));
    _state = GameState::LOADING;
    _fadeAlpha = 0.0f;
    _background->changeTheme(p.level + 1);
    std::cout << "[CLIENT] LEVEL_END : " << p.level << std::endl;
    std::cout << "[CLIENT] Entering LOADING state" << std::endl;
}
