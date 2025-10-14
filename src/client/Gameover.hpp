#pragma once
#include <memory>
#include "engine/renderer/App.hpp"
#include "engine/renderer/Texture.hpp"
#include "engine/events/Events.hpp"

namespace R_Type
{
    class Gameover
    {
        public:
            Gameover(engine::R_Graphic::App &app);
            ~Gameover() = default;
            void draw(bool win);
        private:
            engine::R_Graphic::App &_app;
            std::shared_ptr<engine::R_Graphic::Texture> _background;
            std::vector<std::shared_ptr<engine::R_Graphic::Texture>> _titleLost;
            std::vector<std::shared_ptr<engine::R_Graphic::Texture>> _titleWin;
    };
}