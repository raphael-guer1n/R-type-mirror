#include "Menu.hpp"
#include <iostream>
#include <cstdlib> 
#include <SDL.h>

R_Type::Menu::Menu(engine::R_Graphic::App &app)
    : _app(app)
{
    int winW, winH;
    SDL_GetRendererOutputSize(_app.getWindow().getRenderer(), &winW, &winH);
    _winH = winH;

    // === FONDS ===
    _background = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/menu_bg.png",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(winW, winH)
    );

    _settingsBackground = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/menu_bg.png",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(winW, winH)
    );

    // === MENU PRINCIPAL ===
    _buttonWidth = 400;
    _buttonHeight = 120;
    _centerX = (winW - _buttonWidth) / 2;

    _startButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/start_btn.png",
        engine::R_Graphic::doubleVec2(_centerX, winH / 2 - 150),
        engine::R_Graphic::intVec2(_buttonWidth, _buttonHeight)
    );

    _settingsButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/settings_btn.png",
        engine::R_Graphic::doubleVec2(_centerX, winH / 2),
        engine::R_Graphic::intVec2(_buttonWidth, _buttonHeight)
    );

    _quitButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/quit_btn.png",
        engine::R_Graphic::doubleVec2(_centerX, winH / 2 + 150),
        engine::R_Graphic::intVec2(_buttonWidth, _buttonHeight)
    );

    // === PAGE SETTINGS ===
    int optionSize = 150;
    int centerXOpt = (winW - optionSize) / 2;

    _backButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/back_btn.png",
        engine::R_Graphic::doubleVec2(centerXOpt, winH - optionSize - 100),
        engine::R_Graphic::intVec2(optionSize, optionSize)
    );
}

bool R_Type::Menu::update(const std::vector<engine::R_Events::Event> &events)
{
    for (auto &ev : events) {
        if (ev.type != engine::R_Events::Type::MouseButtonDown)
            continue;

        int x = ev.mouse.x;
        int y = ev.mouse.y;

        if (_currentPage == Page::Main) {
            // Start
            if (x >= _centerX && x <= _centerX + _buttonWidth &&
                y >= _winH / 2 - 150 && y <= _winH / 2 - 150 + _buttonHeight) {
                _startPressed = true;
                return true;
            }

            // Settings
            if (x >= _centerX && x <= _centerX + _buttonWidth &&
                y >= _winH / 2 && y <= _winH / 2 + _buttonHeight) {
                _currentPage = Page::Settings;
            }

            // Quit
            if (x >= _centerX && x <= _centerX + _buttonWidth &&
                y >= _winH / 2 + 150 && y <= _winH / 2 + 150 + _buttonHeight) {
                _quitPressed = true;
                std::exit(0);
            }
        } 
        else if (_currentPage == Page::Settings) {
            int optionSize = 150;
            int centerXOpt = (_app.getWindow().getSize().x - optionSize) / 2;

            // Back
            if (x >= centerXOpt && x <= centerXOpt + optionSize &&
                y >= _winH - optionSize - 100 && y <= _winH - 100) {
                _currentPage = Page::Main;
            }
        }
    }
    return false;
}

void R_Type::Menu::draw()
{
    if (_currentPage == Page::Main)
        drawMainMenu();
    else if (_currentPage == Page::Settings)
        drawSettingsMenu();
}

void R_Type::Menu::drawMainMenu()
{
    _background->draw(_app.getWindow(), nullptr);
    for (auto &tex : _titleLetters)
        tex->draw(_app.getWindow(), nullptr);
    _startButton->draw(_app.getWindow(), nullptr);
    _settingsButton->draw(_app.getWindow(), nullptr);
    _quitButton->draw(_app.getWindow(), nullptr);
}

void R_Type::Menu::drawSettingsMenu()
{
    _settingsBackground->draw(_app.getWindow(), nullptr);
    _backButton->draw(_app.getWindow(), nullptr);
}
