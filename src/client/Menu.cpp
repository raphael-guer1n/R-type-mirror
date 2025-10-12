#include "Menu.hpp"
#include <iostream>
#include <cstdlib>
#include "engine/audio/Music.hpp"

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

    _background = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/menu_bg.png",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(1920, 1080)
    );

    _startButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/start_btn.png",
        engine::R_Graphic::doubleVec2(550, 300),
        engine::R_Graphic::intVec2(400, 120)
    );

    _settingsButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/settings_btn.png",
        engine::R_Graphic::doubleVec2(550, 450),
        engine::R_Graphic::intVec2(400, 120)
    );

    _quitButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/quit_btn.png",
        engine::R_Graphic::doubleVec2(550, 600),
        engine::R_Graphic::intVec2(400, 120)
    );

    float startX = 430.0f; 
    float startY = 60.0f;
    float spacing = 100.0f;
    float scale = 1.5f;

    for (size_t i = 0; i < 6; ++i) {
        std::string title = "R-TYPE";
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

    if (_menuMusic.load("./Assets/Music/Menu.ogg")) {
        _menuMusic.play(true);
    } else {
        std::cerr << "[AUDIO] Failed to load Menu.ogg\n";
    }
}

bool R_Type::Menu::update(const std::vector<engine::R_Events::Event> &events)
{
    for (auto &ev : events) {
        if (ev.type == engine::R_Events::Type::MouseButtonDown) {
            int x = ev.mouse.x;
            int y = ev.mouse.y;

            if (x >= 550 && x <= 950 && y >= 300 && y <= 420) {
                _startPressed = true;

                _menuMusic.stop();

                if (_gameMusic.load("./Assets/Music/Game.ogg")) {
                    _gameMusic.play(true);
                } else {
                    std::cerr << "[AUDIO] Failed to load Game.ogg\n";
                }

                return true;
            }

            if (x >= 550 && x <= 950 && y >= 600 && y <= 720) {
                _quitPressed = true;
                _menuMusic.stop();
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
