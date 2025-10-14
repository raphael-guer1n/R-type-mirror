#include "Enemy.hpp"
#include "Rtype.hpp"

R_Type::Enemy::Enemy(R_Type::Rtype &rtype)
: enemyRect(5, 6, 23, 24), projectileRect(136, 0, 7, 12)
{
    enemyTexture = std::make_shared<engine::R_Graphic::Texture>(
        rtype.getApp().getWindow(),
        "./Assets/sprites/r-typesheet5.gif",
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