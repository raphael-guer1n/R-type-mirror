#include "Enemy.hpp"
#include "Rtype.hpp"
#include "engine/audio/AudioManager.hpp"
#include <iostream>

R_Type::Enemy::Enemy(R_Type::Rtype &rtype)
: _rtypeRef(rtype)
{
    enemyRect = engine::R_Graphic::textureRect(5, 6, 28, 32);
    projectileRect = engine::R_Graphic::textureRect(136, 0, 7, 12);

    enemyTexture = std::make_shared<engine::R_Graphic::Texture>(
        rtype.getApp().getWindow(),
        "./Assets/sprites/r-typesheet23.gif",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(100, 100)
    );

    projectileTexture = std::make_shared<engine::R_Graphic::Texture>(
        rtype.getApp().getWindow(),
        "./Assets/sprites/r-typesheet43.gif",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(100, 100)
    );

    projectileAnimation.clips.insert({
        "idle",
        component::AnimationClip{
            .frameCount = 7,
            .frameTime = 0.12f,
            .startX = 136,
            .startY = 0,
            .frameWidth = 17,
            .frameHeight = 12,
            .loop = true
        }
    });
}


void R_Type::Enemy::setType(const std::string &type)
{
    _enemyType = type;

    std::string path;
    engine::R_Graphic::textureRect rect;
    float scale = 1.0f;

    if (type == "crawler") {
        path = "./Assets/sprites/r-typesheet23.gif";
        rect = engine::R_Graphic::textureRect(5, 6, 28, 32);
        scale = 3.6f;
    }
    else if (type == "shooter") {
        path = "./Assets/sprites/r-typesheet5.gif";
        rect = engine::R_Graphic::textureRect(5, 6, 23, 24);
        scale = 3.5f;
    }
    else if (type == "boss") {
        path = "./Assets/sprites/r-typesheet14.gif";
        rect = engine::R_Graphic::textureRect(5, 6, 50, 58);
        scale = 4.4f;
    }
    else if (type == "spinner") {
        path = "./Assets/sprites/r-typesheet8.gif";
        rect = engine::R_Graphic::textureRect(5, 6, 28, 32);
        scale = 3.5f;
    }
    else if (type == "charger") {
        path = "./Assets/sprites/r-typesheet31.gif";
        rect = engine::R_Graphic::textureRect(5, 6, 29, 40);
        scale = 3.5f;
    }
    else if (type == "boss_laser") {
        path = "./Assets/sprites/r-typesheet9.gif";
        rect = engine::R_Graphic::textureRect(4, 5, 45, 52);
        scale = 8.5f;
    }
    else {
        std::cerr << "[Enemy] Unknown type '" << type << "', using default sprite.\n";
        path = "./Assets/sprites/r-typesheet23.gif";
        rect = engine::R_Graphic::textureRect(5, 6, 28, 32);
        scale = 3.0f;
    }

    engine::R_Graphic::intVec2 scaledSize(
        static_cast<int>(rect.size.x * scale),
        static_cast<int>(rect.size.y * scale)
    );

    enemyTexture = std::make_shared<engine::R_Graphic::Texture>(
        _rtypeRef.getApp().getWindow(),
        path,
        engine::R_Graphic::doubleVec2(0, 0),
        scaledSize
    );

    enemyRect = rect;
}