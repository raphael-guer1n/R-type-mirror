/**
 * @file Hud.hpp
 * @brief Definition of the Hud class used in the R-Type engine.
 *
 * The Hud class represents the Heads-Up Display (HUD) in the R-Type game. 
 * It is responsible for displaying and tracking information visible to 
 * the player (e.g., score, lives, etc.).
 *
 * @details 
 * - The class is tightly coupled with the Rtype class, from which it depends.
 * - It is constructed using a reference to an existing Rtype instance.
 * - The destructor is defaulted.
 *
 * @namespace R_Type
 * Namespace for the R-Type project.
 *
 * @class R_Type::Hud
 * @brief Class managing the in-game HUD (Heads-Up Display) for R-Type.
 *
 * @see R_Type::Rtype
 */

#pragma once
#include "engine/Engine.hpp"
#include "Rtype.hpp"
#include <optional>

namespace R_Type
{
    class Rtype;
    class Hud
    {
    public:
        Hud(R_Type::Rtype &rtype);
        ~Hud() = default;
        void hudUpdate(R_Type::Rtype &rtype);
        void setChargeLevel(R_Type::Rtype &rtype, float level);
        void drawOverlay(R_Type::Rtype &rtype);
        void startLevelAnimation(int level, engine::registry &registry);
        void drawText(std::string &text, float hudScale, float x, float y,
            R_Type::Rtype &rtype, bool level = false, bool score = false);
        void setHealth(std::uint8_t value) {_health = static_cast<int>(value);};
        void setScore(uint32_t value) {_score = static_cast<int>(value);};

    private:
        std::optional<size_t> _chargeFillLocalId;
        engine::registry _registry;
        int _score = 0 ;
        int _health = 0;
        float _chargeLevel = 0.f;
        int _barOriginX = 0;
        int _barOriginY = 0;
        int _barMaxWidth = 420;
        int _barHeight = 20;
        int _levelToDisplay = 0;
        int _levelDisplayTimer = 0;
    };

}