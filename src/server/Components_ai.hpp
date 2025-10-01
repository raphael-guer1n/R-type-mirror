#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace component
{
    struct ai_controller {
        std::string behavior;
        float speed = 2.0f;
        uint32_t shootCooldown = 0;
        uint32_t lastActionTick = 0;
    };

    struct spell {
        std::string name;
        float speedX = 0, speedY = 0;
        uint32_t cooldown = 60;
        uint32_t lastCast = 0;
        int damage = 1;
        float width = 10.f, height = 10.f;
    };

    struct spellbook {
        std::vector<spell> spells;
    };

    struct boss_phase {
        int hpThreshold{0};
        std::string nextAI;
    };
}