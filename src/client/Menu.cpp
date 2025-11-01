#include <iostream>
#include <cstdlib>
#include <string>
#include "Menu.hpp"
#include "Rtype.hpp"
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

    _input = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/input.png",
        engine::R_Graphic::doubleVec2(620, 10),
        engine::R_Graphic::intVec2(550, 150)
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

    _refresh = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/refresh_btn.png",
        engine::R_Graphic::doubleVec2(1770, 10),
        engine::R_Graphic::intVec2(optionSize, optionSize)
    );

    _acceptButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/accept_btn.png",
        engine::R_Graphic::doubleVec2(1200, 10),
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

    if (_menuMusic.load("./Assets/Music/Menu.ogg")) {
        _menuMusic.play(true);
    } else {
        std::cerr << "[AUDIO] Failed to load Menu.ogg\n";
    }
    _font = TTF_OpenFont("./Assets/fonts/arial.ttf", 64);
    if (!_font)
        std::cerr << "Failed to load font\n";
}

bool R_Type::Menu::update(const std::vector<engine::R_Events::Event> &events,
    Rtype& rtype)
{
    for (auto &ev : events) {
        if (_typing && ev.type == engine::R_Events::Type::KeyDown &&
            !_lobbyName.empty() &&
            ev.key.code == engine::R_Events::Key::Backspace) {
            _lobbyName.pop_back();
        } else if (ev.type == engine::R_Events::Type::KeyDown &&
            static_cast<int>(ev.key.code) >= 32 &&
            static_cast<int>(ev.key.code) <= 126) {
            _lobbyName.push_back(static_cast<char>(ev.key.code));
        }
        if (ev.type != engine::R_Events::Type::MouseButtonDown)
            continue;

        int x = ev.mouse.x;
        int y = ev.mouse.y;

        if (_currentPage == Page::Main) {
            if (x >= _centerX && x <= _centerX + _buttonWidth &&
                y >= _winH / 2 - 150 && y <= _winH / 2 - 150 + _buttonHeight) {
                _currentPage = Page::Lobby;
                // _startPressed = true;

                // _menuMusic.stop();

                // if (_gameMusic.load("./Assets/Music/Game.ogg")) {
                //     _gameMusic.play(true);
                // } else {
                //     std::cerr << "[AUDIO] Failed to load Game.ogg\n";
                // }

                // return true;
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
        } else if (_currentPage == Page::Lobby) {
            if (x >= 0 && x <= 150 && y >= 10 && y <= 160) {
                _currentPage = Page::Main;
            }
            if (x >= 620 && x <= 1170 && y >= 10 && y <= 160) {
                _typing = true;
            } else {
                _typing = false;
            }
            if (x >= 1200 && x <= 1350 && y >= 10 && y <= 160) {
                if (!_lobbyName.empty())
                    rtype.createLobby(_lobbyName);
            }
            if (x >= 1770 && x <= 1920 && y >= 10 && y <= 160) {
                std::cout << "REQ LOBBY" << std::endl;
                rtype.requestLobbyList();
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
    else if (_currentPage == Page::Lobby)
        drawLobbyMenu();
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

void R_Type::Menu::drawLobbyMenu()
{
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {240, 225, 60, 255};

    _background->draw(_app.getWindow(), nullptr);
    _refresh->draw(_app.getWindow(), nullptr);
    _backButton->setPosition(0, 10);
    _backButton->draw(_app.getWindow(), nullptr);
    _input->draw(_app.getWindow(), nullptr);
    _acceptButton->draw(_app.getWindow(), nullptr);
    if (!_lobbyName.empty()) {
        if (!_typing)
            drawText(_app.getWindow().getRenderer(), _lobbyName, 650, 44, white);
        else
            drawText(_app.getWindow().getRenderer(), _lobbyName, 650, 44, yellow);
    }
}

void R_Type::Menu::drawText(SDL_Renderer *renderer, const std::string &text, int x, int y, SDL_Color color)
{
    if (!_font) return;

    SDL_Surface* surface = TTF_RenderText_Solid(_font, text.c_str(), color);
    if (!surface) {
        std::cerr << "RenderText failed: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect destRect = { x, y, surface->w, surface->h };
    SDL_RenderCopy(renderer, texture, nullptr, &destRect);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}
