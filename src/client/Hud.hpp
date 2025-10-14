#pragma once

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
        void setChargeLevel(R_Type::Rtype &rtype, float level);
        void drawOverlay(R_Type::Rtype &rtype);

    private:
        std::optional<size_t> _chargeFillLocalId;
        float _chargeLevel = 0.f;
        int _barOriginX = 0;
        int _barOriginY = 0;
        int _barMaxWidth = 420;
        int _barHeight = 20;
    };

}