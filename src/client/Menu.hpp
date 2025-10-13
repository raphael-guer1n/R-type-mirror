#pragma once
#include <memory>
#include "engine/renderer/App.hpp"
#include "engine/renderer/Texture.hpp"
#include "engine/events/Events.hpp"

namespace R_Type {
    class Menu {
    public:
        explicit Menu(engine::R_Graphic::App &app);
        ~Menu() = default;

        bool update(const std::vector<engine::R_Events::Event> &events);

        void draw();

    private:
        engine::R_Graphic::App &_app;
        std::shared_ptr<engine::R_Graphic::Texture> _background;
        std::shared_ptr<engine::R_Graphic::Texture> _startButton;
        std::shared_ptr<engine::R_Graphic::Texture> _quitButton;
        std::shared_ptr<engine::R_Graphic::Texture> _settingsButton;
        std::vector<std::shared_ptr<engine::R_Graphic::Texture>> _titleLetters;

        bool _startPressed = false;
        bool _quitPressed = false;
    };
}
