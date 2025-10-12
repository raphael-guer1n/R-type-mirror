#include <memory>
#include "Rtype.hpp"
#include "Player.hpp"

R_Type::Player::Player(R_Type::Rtype &rtype)
: playerRect(0, 0, 33, 17), projectileRect(232, 103, 16, 12)
{
    playerTexture = std::make_shared<engine::R_Graphic::Texture>(
        rtype.getApp().getWindow(),
        "./Assets/sprites/r-typesheet42.gif",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(132, 72)
    );
    projectileTexture = std::make_shared<engine::R_Graphic::Texture>(
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
            .startX = 66,
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
            .startX = 66,
            .startY = 0,
            .frameWidth = 33,
            .frameHeight = 17,
            .loop = false
        }
    });
    playerAnimation.clips.insert({
        "move_down",
        component::AnimationClip{
            .frameCount = 3,
            .frameTime = 0.12f,
            .startX = 66,
            .startY = 0,
            .frameWidth = 33,
            .frameHeight = 17,
            .loop = false
        }
    });
    projectileAnimation.clips.insert({
        "idle",
        component::AnimationClip{
            .frameCount = 2,
            .frameTime = 0.12f,
            .startX = 232,
            .startY = 103,
            .frameWidth = 17,
            .frameHeight = 12,
            .loop = true
        }
    });
}

void R_Type::Player::playerUpdateAnimation(std::unordered_map<uint32_t, size_t>& entityMap,
    uint32_t player, engine::registry& registry, const std::unordered_set<engine::R_Events::Key>& pressedKeys)
{
    auto it = entityMap.find(player);
    if (it != entityMap.end()) {
        size_t localId = it->second;
        auto& animations = registry.get_components<component::animation>();
        if (localId < animations.size() && animations[localId].has_value()) {
            auto &anim = *animations[localId];
            using engine::R_Events::Key;
            if (pressedKeys.count(Key::Up)) {
                setAnimation(anim, "move_up", false);
            } else if (pressedKeys.count(Key::Down)) {
                setAnimation(anim, "move_down", true);
            }
            else {
                setAnimation(anim, "idle", false);
            }
        }
    }
}
