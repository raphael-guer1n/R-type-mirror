#include <memory>
#include "Rtype.hpp"
#include "Player.hpp"

R_Type::Player::Player(R_Type::Rtype &rtype)
: playerRect(167, 0, 32, 17), projectileRect(225, 97, 24, 20)
{
    texture = std::make_shared<engine::R_Graphic::Texture>(
        rtype.getApp().getWindow(),
        "./Assets/sprites/r-typesheet1.gif",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(132, 72)
    );
    playerAnimation.clips.insert({
        "idle",
        component::AnimationClip{
            .frameCount = 1,
            .frameTime = 0.0f,
            .startX = 167,
            .startY = 0,
            .frameWidth = 33,
            .frameHeight = 17,
            .loop = false
        }
    });
    playerAnimation.clips.insert({
        "move_up",
        component::AnimationClip{
            .frameCount = 3,
            .frameTime = 0.12f,
            .startX = 167,
            .startY = 0,
            .frameWidth = 33,
            .frameHeight = 17,
            .loop = false
        }
    });
}
