#include "Menu.hpp"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>
#include "engine/audio/Music.hpp"
#include "common/Accessibility.hpp"
#include "engine/audio/AudioManager.hpp"
#include <SDL_ttf.h>

using json = nlohmann::json;

static inline bool hit(int x, int y, int rx, int ry, int rw, int rh) {
    return (x >= rx && x <= rx + rw && y >= ry && y <= ry + rh);
}

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
    _buttonWidth  = 400;
    _buttonHeight = 120;
    _centerX      = (winW - _buttonWidth) / 2;

    const int startY  = winH / 2 - 150;
    const int settY   = winH / 2;
    const int helpY   = winH / 2 + 150;
    const int quitY   = winH / 2 + 300;
    const int accY    = winH / 2 + 450;

    _background = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(), "./Assets/Menu/menu_bg.png",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(winW, winH)
    );

    _settingsBackground = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(), "./Assets/Menu/menu_bg.png",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(winW, winH)
    );

    _startButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(), "./Assets/Menu/start_btn.png",
        engine::R_Graphic::doubleVec2(_centerX, startY),
        engine::R_Graphic::intVec2(_buttonWidth, _buttonHeight)
    );

    _settingsButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(), "./Assets/Menu/settings_btn.png",
        engine::R_Graphic::doubleVec2(_centerX, settY),
        engine::R_Graphic::intVec2(_buttonWidth, _buttonHeight)
    );

    _helpButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(), "./Assets/Menu/help_btn.png",
        engine::R_Graphic::doubleVec2(_centerX, helpY),
        engine::R_Graphic::intVec2(_buttonWidth, _buttonHeight)
    );

    _quitButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(), "./Assets/Menu/quit_btn.png",
        engine::R_Graphic::doubleVec2(_centerX, quitY),
        engine::R_Graphic::intVec2(_buttonWidth, _buttonHeight)
    );

    _accessibilityButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(), "./Assets/Menu/accessibility_btn.png",
        engine::R_Graphic::doubleVec2(_centerX, accY),
        engine::R_Graphic::intVec2(_buttonWidth, _buttonHeight)
    );
    int optionSize   = 150;
    int spacingSmall = 70;
    int totalWidth   = (3 * optionSize) + 2 * spacingSmall;
    int startX       = (winW - totalWidth) / 2;
    int y            = winH / 2 - optionSize / 2;

    _backButtonPos = engine::R_Graphic::doubleVec2(startX, y);
    _backButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(), "./Assets/Menu/back_btn.png",
        _backButtonPos, engine::R_Graphic::intVec2(optionSize, optionSize)
    );

    _soundButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(), "./Assets/Menu/sound_btn.png",
        engine::R_Graphic::doubleVec2(startX + optionSize + spacingSmall, y),
        engine::R_Graphic::intVec2(optionSize, optionSize)
    );

    _muteButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(), "./Assets/Menu/mute_btn.png",
        engine::R_Graphic::doubleVec2(startX + optionSize + spacingSmall, y),
        engine::R_Graphic::intVec2(optionSize, optionSize)
    );

    _windowButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(), "./Assets/Menu/window_btn.png",
        engine::R_Graphic::doubleVec2(startX + (2 * optionSize) + 2 * spacingSmall, y),
        engine::R_Graphic::intVec2(optionSize, optionSize)
    );
    std::string title = "R-TYPE";
    float spacing = 100.0f;
    float scale = 1.5f;

    float titleWidth = (title.size() - 1) * spacing + 128 * scale;
    float titleX = (winW - titleWidth) / 2;
    float titleY = (winH / 2.f) - 400.0f;

    for (size_t i = 0; i < title.size(); ++i) {
        char ch = std::toupper(title[i]);
        if (!fontMap.count(ch)) continue;

        std::string path = "./Assets/Hud/Score/" + fontMap[ch];
        auto tex = std::make_shared<engine::R_Graphic::Texture>(
            _app.getWindow(), path,
            engine::R_Graphic::doubleVec2(titleX + i * spacing, titleY),
            engine::R_Graphic::intVec2(static_cast<int>(128 * scale), static_cast<int>(128 * scale))
        );
        _titleLetters.push_back(tex);
    }
    if (_menuMusic.load("./Assets/Music/Menu.wav")) {
        _menuMusic.setMuted(!_soundEnabled);
        if (_soundEnabled) _menuMusic.play(true);
    } else {
        std::cerr << "[AUDIO] Failed to load Menu.wav\n";
    }
}

bool R_Type::Menu::update(const std::vector<engine::R_Events::Event> &events)
{
    for (auto &ev : events) {
        if (ev.type == engine::R_Events::Type::Quit ||
            (ev.type == engine::R_Events::Type::KeyDown &&
             ev.key.code == engine::R_Events::Key::Escape))
        {
            std::cout << "[SYSTEM] Quit requested.\n";
            _menuMusic.stop();
            _gameMusic.stop();
            SDL_Quit();
            std::exit(0);
        }
        if (ev.type != engine::R_Events::Type::MouseButtonDown)
            continue;
        int x = ev.mouse.x;
        int y = ev.mouse.y;
        if (_currentPage == Page::Main) {
            const int startY = _winH / 2 - 150;
            const int settY  = _winH / 2;
            const int helpY  = _winH / 2 + 150;
            const int quitY  = _winH / 2 + 300;
            const int accY   = _winH / 2 + 450;
            if (hit(x, y, _centerX, startY, _buttonWidth, _buttonHeight)) {
                _startPressed = true;
                _menuMusic.stop();
                if (_gameMusic.load("./Assets/Music/Game.wav")) {
                    _gameMusic.setMuted(!_soundEnabled);
                    if (_soundEnabled) _gameMusic.play(true);
                } else {
                    std::cerr << "[AUDIO] Failed to load Game.wav\n";
                }
                return true;
            }
            if (hit(x, y, _centerX, settY, _buttonWidth, _buttonHeight)) {
                _currentPage = Page::Settings;
                return false;
            }
            if (hit(x, y, _centerX, helpY, _buttonWidth, _buttonHeight)) {
                _currentPage = Page::Help;
                return false;
            }
            if (hit(x, y, _centerX, quitY, _buttonWidth, _buttonHeight)) {
                _quitPressed = true;
                _menuMusic.stop();
                std::exit(0);
            }
            if (hit(x, y, _centerX, accY, _buttonWidth, _buttonHeight)) {
                AccessibilityConfig::load_from_json("configs/accessibility_config.json");
                AccessibilityConfig::enabled = true;
                engine::audio::AudioManager::instance().setMusicVolume(AccessibilityConfig::volume_general);
                engine::audio::AudioManager::instance().setSFXVolume(AccessibilityConfig::volume_general);
                return false;
            }
        }
        else if (_currentPage == Page::Settings)
        {
            int optionSize   = 150;
            int spacingSmall = 70;
            int totalWidth   = (3 * optionSize) + 2 * spacingSmall;
            int startX       = (_app.getWindow().getSize().x - totalWidth) / 2;
            int yBtn         = _winH / 2 - optionSize / 2;

            if (hit(x, y, startX, yBtn, optionSize, optionSize)) {
                _currentPage = Page::Main;
                return false;
            }
            if (hit(x, y, startX + optionSize + spacingSmall, yBtn, optionSize, optionSize)) {
                _soundEnabled = !_soundEnabled;
                _menuMusic.setMuted(!_soundEnabled);
                _gameMusic.setMuted(!_soundEnabled);
                if (_soundEnabled) {
                    std::cout << " Music resumed\n";
                    _menuMusic.resume();
                } else {
                    std::cout << " Music paused\n";
                    _menuMusic.pause();
                }
                return false;
            }
            if (hit(x, y, startX + (2 * optionSize) + 2 * spacingSmall, yBtn, optionSize, optionSize)) {
                _fullscreen = !_fullscreen;
                SDL_Window *sdlWindow = _app.getWindow().getWindow();
                if (_fullscreen)
                    SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN);
                else {
                    SDL_SetWindowFullscreen(sdlWindow, 0);
                    SDL_SetWindowSize(sdlWindow, 1920, 1080);
                    SDL_SetWindowPosition(sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                }
                return false;
            }
        }
        else if (_currentPage == Page::Help)
        {
            if (ev.type == engine::R_Events::Type::MouseButtonDown)
            {
                _currentPage = Page::Main;
            }
        }
    }
    return false;
}

void R_Type::Menu::draw()
{
    if (_currentPage == Page::Main)       drawMainMenu();
    else if (_currentPage == Page::Settings) drawSettingsMenu();
    else if (_currentPage == Page::Help)     drawHelpMenu();
}

void R_Type::Menu::drawMainMenu()
{
    _background->draw(_app.getWindow(), nullptr);
    for (auto &tex : _titleLetters)
        tex->draw(_app.getWindow(), nullptr);
    _startButton->draw(_app.getWindow(), nullptr);
    _settingsButton->draw(_app.getWindow(), nullptr);
    _helpButton->draw(_app.getWindow(), nullptr);
    _quitButton->draw(_app.getWindow(), nullptr);
    _accessibilityButton->draw(_app.getWindow(), nullptr);
}

void R_Type::Menu::drawSettingsMenu()
{
    _settingsBackground->draw(_app.getWindow(), nullptr);
    _backButton->draw(_app.getWindow(), nullptr);
    if (_soundEnabled)
        _soundButton->draw(_app.getWindow(), nullptr);
    else
        _muteButton->draw(_app.getWindow(), nullptr);
    _windowButton->draw(_app.getWindow(), nullptr);
}

void R_Type::Menu::drawHelpMenu()
{
    _settingsBackground->draw(_app.getWindow(), nullptr);

    SDL_Renderer* ren = _app.getWindow().getRenderer();
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 200);
    SDL_Rect overlay = {0, 0, 1920, 1080};
    SDL_RenderFillRect(ren, &overlay);

    TTF_Font* fontTitle = TTF_OpenFont("./Assets/Fonts/arial.ttf", 60);
    TTF_Font* fontBody  = TTF_OpenFont("./Assets/Fonts/arial.ttf", 36);

    if (!fontTitle || !fontBody) {
        if (fontTitle) TTF_CloseFont(fontTitle);
        if (fontBody)  TTF_CloseFont(fontBody);
        _backButton->draw(_app.getWindow(), nullptr);
        return;
    }

    SDL_Color white  = {255, 255, 255, 255};
    SDL_Color yellow = {255, 220, 80, 255};

    SDL_Surface* titleSurf = TTF_RenderUTF8_Blended(fontTitle, "=== PLayer's Rules ===", yellow);
    if (titleSurf) {
        SDL_Texture* titleTex = SDL_CreateTextureFromSurface(ren, titleSurf);
        SDL_Rect dst = {500, 100, titleSurf->w, titleSurf->h};
        SDL_RenderCopy(ren, titleTex, nullptr, &dst);
        SDL_DestroyTexture(titleTex);
        SDL_FreeSurface(titleSurf);
    }

    const char* helpText =
        "MAIN CONTROLS:\n"
        "  → Movement: Arrow keys\n"
        "  → Normal shot: Space bar\n"
        "  → Special shot: Right Shift (hold to charge)\n"
        "  → Quit game: ESC key\n"
        "\n"
        "ACCESSIBILITY:\n"
        "  • High contrast mode to better distinguish sprites\n"
        "  • Adjustable game speed for a slower pace\n"
        "  • Global volume automatically adapted\n"
        "  • Remappable controls via 'configs/accessibility_config.json'\n"
        "\n"
        "TIPS:\n"
        "  • Enable accessibility mode in the main menu.\n"
        "  • You can exit anytime with 'ESC'.\n"
        "  • The HUD can adjust its size to your needs.\n"
        "Click anywhere to return to the main menu.";
    SDL_Surface* bodySurf = TTF_RenderUTF8_Blended_Wrapped(fontBody, helpText, white, 1600);
    if (bodySurf) {
        SDL_Texture* bodyTex = SDL_CreateTextureFromSurface(ren, bodySurf);
        SDL_Rect dst = {160, 250, bodySurf->w, bodySurf->h};
        SDL_RenderCopy(ren, bodyTex, nullptr, &dst);
        SDL_DestroyTexture(bodyTex);
        SDL_FreeSurface(bodySurf);
    }

    TTF_CloseFont(fontTitle);
    TTF_CloseFont(fontBody);
}
