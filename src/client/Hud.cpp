#include "Hud.hpp"
#include "common/Components_client_sdl.hpp"
#include "Rtype.hpp"
#include "common/Layers.hpp"
#include <memory>
#include <unordered_map>
#include <cctype>
#include <SDL.h>

static std::unordered_map<char, std::string> fontMap = {
    {'0', "CK_StarGlowing_0.png"}, {'1', "CK_StarGlowing_1.png"},
    {'2', "CK_StarGlowing_2.png"}, {'3', "CK_StarGlowing_3.png"},
    {'4', "CK_StarGlowing_4.png"}, {'5', "CK_StarGlowing_5.png"},
    {'6', "CK_StarGlowing_6.png"}, {'7', "CK_StarGlowing_7.png"},
    {'8', "CK_StarGlowing_8.png"}, {'9', "CK_StarGlowing_9.png"},
    {'C', "CK_StarGlowing_C.png"}, {'E', "CK_StarGlowing_E.png"},
    {'O', "CK_StarGlowing_O.png"}, {'R', "CK_StarGlowing_R.png"},
    {'P', "CK_StarGlowing_P.png"}, {'H', "CK_StarGlowing_H.png"},
    {'I', "CK_StarGlowing_I.png"}, {'S', "CK_StarGlowing_S.png"}
};

R_Type::Hud::Hud(R_Type::Rtype &rtype)
{
    auto &registry = rtype.getRegistry();
    auto &window = rtype.getApp().getWindow();

    int winW = 0, winH = 0;
    SDL_GetRendererOutputSize(window.getRenderer(), &winW, &winH);

    registry.register_component<component::beam_charge>();
    registry.register_component<component::score>();
    registry.register_component<component::health>();
    registry.register_component<component::hud_tag>();

    // === BEAM BAR ===
    auto e = registry.spawn_entity();
    float barWidth = 500.0f;
    float barHeight = 120.0f;
    float barX = (winW - barWidth) / 2.0f;
    float barY = winH - barHeight - 60.0f;
    engine::R_Graphic::doubleVec2 hudPos(barX, barY);

    registry.add_component(e, component::position{barX, barY});
    registry.add_component(e, component::beam_charge{0.0f, false});
    registry.add_component(e, component::score{15200, 99999});
    registry.add_component(e, component::hud_tag{});

    auto bar = std::make_shared<engine::R_Graphic::Texture>(
        window,
        "./Assets/Hud/beam_frame.png",
        hudPos,
        engine::R_Graphic::intVec2(500, 120));

    engine::R_Graphic::textureRect rect(0, 530, 1200, 140);
    registry.emplace_component<component::drawable>(e, bar, rect, layers::HudBase);

    // === SCORE ===
    float hudScale = 0.5f;
    int scoreValue = 200;
    std::string scoreText = "1P " + std::to_string(scoreValue);
    float spacing = 33.0f;
    float startY = 55.0f;
    float totalWidth = scoreText.size() * spacing;
    float startX = winW - totalWidth - 110.0f;

    engine::R_Graphic::intVec2 scaledSize(
        static_cast<int>(128 * hudScale),
        static_cast<int>(128 * hudScale));

    for (size_t i = 0; i < scoreText.size(); ++i)
    {
        char ch = std::toupper(scoreText[i]);
        if (!fontMap.count(ch))
            continue;

        std::string path = "./Assets/Hud/Score/" + fontMap[ch];
        auto digitEntity = registry.spawn_entity();

        float posX = startX + i * spacing;

        registry.add_component(digitEntity, component::position{posX, startY});
        registry.add_component(digitEntity, component::hud_tag{});

        auto tex = std::make_shared<engine::R_Graphic::Texture>(
            window,
            path,
            engine::R_Graphic::doubleVec2(posX, startY),
            scaledSize);

        engine::R_Graphic::textureRect rectDigit(0, 0, 128, 128);
        registry.emplace_component<component::drawable>(digitEntity, tex, rectDigit, layers::HudText);
    }

    // === HIGH SCORE ===
    int highScoreValue = 99999;
    std::string highScoreText = "HI " + std::to_string(highScoreValue);
    float highStartY = 10.0f;
    float highWidth = highScoreText.size() * spacing;
    float highStartX = winW - highWidth - 40.0f;

    for (size_t i = 0; i < highScoreText.size(); ++i)
    {
        char ch = std::toupper(highScoreText[i]);
        if (!fontMap.count(ch))
            continue;

        std::string path = "./Assets/Hud/Score/" + fontMap[ch];
        auto highEntity = registry.spawn_entity();

        float posX = highStartX + i * spacing;

        registry.add_component(highEntity, component::position{posX, highStartY});
        registry.add_component(highEntity, component::hud_tag{});

        auto tex = std::make_shared<engine::R_Graphic::Texture>(
            window,
            path,
            engine::R_Graphic::doubleVec2(posX, highStartY),
            scaledSize);

        engine::R_Graphic::textureRect rectHigh(0, 0, 128, 128);
        registry.emplace_component<component::drawable>(highEntity, tex, rectHigh, layers::HudText);
    }

    // === HEARTS ===
    int maxHearts = 3;
    std::uint8_t currentHP = 3;
    float heartY = winH - 80.0f;
    float heartSpacing = 45.0f;
    float heartSize = 32.0f;
    float totalHeartsWidth = (currentHP - 1) * heartSpacing + heartSize;
    float heartX = (winW - totalHeartsWidth) / 2.0f;

    for (std::uint8_t i = 0; i < currentHP && i < maxHearts; ++i)
    {
        auto heartEntity = registry.spawn_entity();
        float posX = heartX + i * heartSpacing;

        registry.add_component(heartEntity, component::position{posX, heartY});
        registry.add_component(heartEntity, component::hud_tag{});
        registry.add_component(heartEntity, component::health{1});

        auto heartTex = std::make_shared<engine::R_Graphic::Texture>(
            window,
            "./Assets/Hud/heart.png",
            engine::R_Graphic::doubleVec2(posX, heartY),
            engine::R_Graphic::intVec2(32, 32));

        engine::R_Graphic::textureRect heartRect(0, 0, 32, 32);
        registry.emplace_component<component::drawable>(heartEntity, heartTex, heartRect, layers::HudIcons);
    }
}
