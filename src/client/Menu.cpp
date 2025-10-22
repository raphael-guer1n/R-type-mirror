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

    int winW, winH;
    SDL_GetRendererOutputSize(_app.getWindow().getRenderer(), &winW, &winH);
    _winH = winH;
    _soundEnabled = true;
    _fullscreen = false;

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
    int spacingSmall = 70;
    int totalWidth = (3 * optionSize) + spacingSmall + spacingSmall;
    int startX = (winW - totalWidth) / 2;
    int y = winH / 2 - optionSize / 2;

    _backButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/back_btn.png",
        engine::R_Graphic::doubleVec2(startX, y),
        engine::R_Graphic::intVec2(optionSize, optionSize)
    );

    _soundButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/sound_btn.png",
        engine::R_Graphic::doubleVec2(startX + optionSize + spacingSmall, y),
        engine::R_Graphic::intVec2(optionSize, optionSize)
    );

    _windowButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/window_btn.png",
        engine::R_Graphic::doubleVec2(startX + (2 * optionSize) + spacingSmall + spacingSmall, y),
        engine::R_Graphic::intVec2(optionSize, optionSize)
    );

    // === TITRE "R-TYPE" ===
    std::string title = "R-TYPE";
    float spacing = 100.0f;
    float scale = 1.5f;

    float titleWidth = (title.size() - 1) * spacing + 128 * scale;
    float titleX = (winW - titleWidth) / 2;
    float titleY = (winH / 2) - 400.0f;

    for (size_t i = 0; i < title.size(); ++i) {
        char ch = std::toupper(title[i]);
        if (!fontMap.count(ch))
            continue;

        std::string path = "./Assets/Hud/Score/" + fontMap[ch];
        auto tex = std::make_shared<engine::R_Graphic::Texture>(
            _app.getWindow(),
            path,
            engine::R_Graphic::doubleVec2(titleX + i * spacing, titleY),
            engine::R_Graphic::intVec2(static_cast<int>(128 * scale), static_cast<int>(128 * scale))
        );
        _titleLetters.push_back(tex);
    }

    if (_menuMusic.load("./Assets/Music/Menu.wav")) {
        _menuMusic.play(true);
    } else {
        std::cerr << "[AUDIO] Failed to load Menu.wav\n";
    }
}

bool R_Type::Menu::update(const std::vector<engine::R_Events::Event> &events)
{
    for (auto &ev : events) {
        if (ev.type != engine::R_Events::Type::MouseButtonDown)
            continue;

        int x = ev.mouse.x;
        int y = ev.mouse.y;

        if (_currentPage == Page::Main) {
            if (x >= _centerX && x <= _centerX + _buttonWidth &&
                y >= _winH / 2 - 150 && y <= _winH / 2 - 150 + _buttonHeight) {
                _startPressed = true;

                _menuMusic.stop();

                if (_gameMusic.load("./Assets/Music/Game.wav")) {
                    _gameMusic.play(true);
                } else {
                    std::cerr << "[AUDIO] Failed to load Game.wav\n";
                }

                return true;
            }

            if (x >= _centerX && x <= _centerX + _buttonWidth &&
                y >= _winH / 2 && y <= _winH / 2 + _buttonHeight)
            {
                _currentPage = Page::Settings;
            }

            if (x >= 550 && x <= 950 && y >= 600 && y <= 720) {
                _quitPressed = true;
                _menuMusic.stop();
                std::exit(0);
            }
        }
        else if (_currentPage == Page::Settings)
        {
            int optionSize = 150;
            int spacingSmall = 70;
            int totalWidth = (3 * optionSize) + spacingSmall + spacingSmall;
            int startX = (_app.getWindow().getSize().x - totalWidth) / 2;
            int yBtn = _winH / 2 - optionSize / 2;

            // === BACK ===
            if (x >= startX && x <= startX + optionSize &&
                y >= yBtn && y <= yBtn + optionSize)
            {
                _currentPage = Page::Main;
            }

            // === SOUND ===
            if (x >= startX + optionSize + spacingSmall &&
                x <= startX + optionSize + spacingSmall + optionSize &&
                y >= yBtn && y <= yBtn + optionSize)
            {
                _soundEnabled = !_soundEnabled;
                if (_soundEnabled)
                    SDL_PauseAudio(0);
                else
                    SDL_PauseAudio(1);
            }

            // === WINDOW ===
            if (x >= startX + (2 * optionSize) + spacingSmall + spacingSmall &&
                x <= startX + (2 * optionSize) + spacingSmall + spacingSmall + optionSize &&
                y >= yBtn && y <= yBtn + optionSize)
            {
                _fullscreen = !_fullscreen;
                SDL_Window *sdlWindow = _app.getWindow().getWindow();
                if (_fullscreen)
                    SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN);
                else {
                    SDL_SetWindowFullscreen(sdlWindow, 0);
                    SDL_SetWindowSize(sdlWindow, 1920, 1080);
                    SDL_SetWindowPosition(sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                }
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
    _soundButton->draw(_app.getWindow(), nullptr);
    _windowButton->draw(_app.getWindow(), nullptr);
}