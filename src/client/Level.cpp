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
