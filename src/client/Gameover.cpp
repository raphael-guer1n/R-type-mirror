#include <memory>
#include "Gameover.hpp"

static std::unordered_map<char, std::string> fontMap = {
    {'Y', "CK_StarGlowing_Y.png"}, {'W', "CK_StarGlowing_W.png"},
    {'O', "CK_StarGlowing_O.png"}, {'N', "CK_StarGlowing_N.png"},
    {'U', "CK_StarGlowing_U.png"}, {'T', "CK_StarGlowing_T.png"},
    {'I', "CK_StarGlowing_I.png"}, {'S', "CK_StarGlowing_S.png"},
    {'L', "CK_StarGlowing_L.png"}
};
R_Type::Gameover::Gameover(engine::R_Graphic::App &app)
    : _app(app)
{
    _background = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/menu_bg.png",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(1920, 1080)
    );

    float startX = 1920.0f / 2 - 500;
    float startY = 1080.0f / 2 - 180;
    float spacing = 100.0f;
    float scale = 1.5f;

    for (size_t i = 0; i < 9; ++i) {
        std::string title = "You Lost";
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
        _titleLost.push_back(tex);
    }
    for (size_t i = 0; i < 8; ++i) {
        std::string title = "You Win";
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
        _titleWin.push_back(tex);
    }
}

void R_Type::Gameover::draw(bool win)
{
    _background->draw(_app.getWindow(), nullptr);
    if (win) {
        for (auto &tex : _titleWin)
            tex->draw(_app.getWindow(), nullptr);
    } else {
        for (auto &tex : _titleLost)
            tex->draw(_app.getWindow(), nullptr);
    }
}
