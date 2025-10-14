#include "Menu.hpp"
#include <iostream>
#include <cstdlib> 
#include <SDL.h>

R_Type::Menu::Menu(engine::R_Graphic::App &app)
    : _app(app)
{
    static std::unordered_map<char, std::string> fontMap = {
        {'R', "CK_StarGlowing_R.png"},
        {'-', "CK_StarGlowing_-.png"},
        {'T', "CK_StarGlowing_T.png"},
        {'Y', "CK_StarGlowing_Y.png"},
        {'P', "CK_StarGlowing_P.png"},
        {'E', "CK_StarGlowing_E.png"}
    };

    int winW = 0, winH = 0;
    SDL_GetRendererOutputSize(_app.getWindow().getRenderer(), &winW, &winH);

    _background = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/menu_bg.png",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(winW, winH)
    );

    int buttonWidth = 400;
    int buttonHeight = 120;
    int centerX = (winW - buttonWidth) / 2;

    _startButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/start_btn.png",
        engine::R_Graphic::doubleVec2(centerX, winH / 2 - 150),
        engine::R_Graphic::intVec2(buttonWidth, buttonHeight)
    );

    _settingsButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/settings_btn.png",
        engine::R_Graphic::doubleVec2(centerX, winH / 2),
        engine::R_Graphic::intVec2(buttonWidth, buttonHeight)
    );

    _quitButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/quit_btn.png",
        engine::R_Graphic::doubleVec2(centerX, winH / 2 + 150),
        engine::R_Graphic::intVec2(buttonWidth, buttonHeight)
    );

    std::string title = "R-TYPE";
    float spacing = 100.0f;
    float scale = 1.5f;
    float titleWidth = title.size() * spacing;
    float startX = (winW - titleWidth) / 2;
    float startY = 60.0f;

    for (size_t i = 0; i < title.size(); ++i) {
        char ch = std::toupper(title[i]);
        if (!fontMap.count(ch))
            continue;

        std::string path = "./Assets/Hud/Score/" + fontMap[ch];
        auto tex = std::make_shared<engine::R_Graphic::Texture>(
            _app.getWindow(),
            path,
            engine::R_Graphic::doubleVec2(startX + i * spacing, startY),
            engine::R_Graphic::intVec2(static_cast<int>(128 * scale), static_cast<int>(128 * scale))
        );
        _titleLetters.push_back(tex);
    }

    _buttonWidth = buttonWidth;
    _buttonHeight = buttonHeight;
    _centerX = centerX;
    _winH = winH;
}

bool R_Type::Menu::update(const std::vector<engine::R_Events::Event> &events)
{
    for (auto &ev : events) {
        if (ev.type == engine::R_Events::Type::MouseButtonDown) {
            int x = ev.mouse.x;
            int y = ev.mouse.y;

            if (x >= _centerX && x <= _centerX + _buttonWidth &&
                y >= _winH / 2 - 150 && y <= _winH / 2 - 150 + _buttonHeight) {
                _startPressed = true;
                return true;
            }

            if (x >= _centerX && x <= _centerX + _buttonWidth &&
                y >= _winH / 2 + 150 && y <= _winH / 2 + 150 + _buttonHeight) {
                _quitPressed = true;
                std::exit(0);
            }
        }
    }
    return false;
}

void R_Type::Menu::draw()
{
    _background->draw(_app.getWindow(), nullptr);
    for (auto &tex : _titleLetters)
        tex->draw(_app.getWindow(), nullptr);
    _startButton->draw(_app.getWindow(), nullptr);
    _settingsButton->draw(_app.getWindow(), nullptr);
    _quitButton->draw(_app.getWindow(), nullptr);
}
