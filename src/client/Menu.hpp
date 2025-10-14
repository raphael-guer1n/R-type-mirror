#pragma once

#include "engine/renderer/App.hpp"
#include "engine/renderer/Texture.hpp"
#include "engine/events/Events.hpp"
#include <vector>
#include <memory>

namespace R_Type {

class Menu {
public:
    explicit Menu(engine::R_Graphic::App &app);
    ~Menu() = default;

    bool update(const std::vector<engine::R_Events::Event> &events);
    void draw();

private:
    enum class Page {
        Main,
        Settings
    };

    void drawMainMenu();
    void drawSettingsMenu();

    engine::R_Graphic::App &_app;
    Page _currentPage = Page::Main;

    std::shared_ptr<engine::R_Graphic::Texture> _background;
    std::shared_ptr<engine::R_Graphic::Texture> _settingsBackground;

    std::shared_ptr<engine::R_Graphic::Texture> _startButton;
    std::shared_ptr<engine::R_Graphic::Texture> _settingsButton;
    std::shared_ptr<engine::R_Graphic::Texture> _quitButton;

    std::shared_ptr<engine::R_Graphic::Texture> _backButton;
    std::shared_ptr<engine::R_Graphic::Texture> _soundButton;
    std::shared_ptr<engine::R_Graphic::Texture> _windowButton;

    std::vector<std::shared_ptr<engine::R_Graphic::Texture>> _titleLetters;

    bool _startPressed = false;
    bool _quitPressed = false;

    bool _soundEnabled = true;
    bool _fullscreen = false;
    int _buttonWidth = 0;
    int _buttonHeight = 0;
    int _centerX = 0;
    int _winH = 0;
};
}