#include <memory>
#include "Rtype.hpp"
#include "engine/renderer/Error.hpp"
#include "Player.hpp"
#include "common/Layers.hpp"

R_Type::Player::Player(R_Type::Rtype &rtype)
: playerRect(0, 0, 33, 17), projectileRect(232, 103, 16, 12), chargeRect(0, 5, 32, 32), chargeProjectileRect(203, 276, 220, 287), missileProjectileRect(0, 238, 152, 254)
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
    chargeTexture = std::make_shared<engine::R_Graphic::Texture>(
        rtype.getApp().getWindow(),
        "./Assets/sprites/r-typesheet1.gif",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(132, 72)
    );
    chargeProjectileTexture = std::make_shared<engine::R_Graphic::Texture>(
        rtype.getApp().getWindow(),
        "./Assets/sprites/r-typesheet1.gif",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(123, 72)
    );
    missileProjectileTexture = std::make_shared<engine::R_Graphic::Texture>(
        rtype.getApp().getWindow(),
        "./Assets/sprites/r-typesheet1.gif",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(123, 72)
    );

    chargeAnimation.clips.insert({
        "charge",
        component::AnimationClip{
            .frameCount = 8,
            .frameTime = 0.06f,
            .startX = 0,
            .startY = 51,
            .frameWidth = 32,
            .frameHeight = 32,
            .loop = true
        }
    });

    chargeProjectileAnimation.clips.insert({
        "idle",
        component::AnimationClip{
            .frameCount = 4,
            .frameTime = 0.06f,
            .startX = 203,
            .startY = 276,
            .frameWidth = 18,
            .frameHeight = 12,
            .loop = true
        }
    });

    missileProjectileAnimation.clips.insert({
        "idle",
        component::AnimationClip{
            .frameCount = 9,
            .frameTime = 0.06f,
            .startX = 0,
            .startY = 238,
            .frameWidth = 16,
            .frameHeight = 16,
            .loop = true
        }
    });

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
            if (pressedKeys.count(Key::Up) or pressedKeys.count(Key::Z)) {
                setAnimation(anim, "move_up", false);
            } else if (pressedKeys.count(Key::Down) or pressedKeys.count(Key::S)) {
                setAnimation(anim, "move_down", true);
            }
            else {
                setAnimation(anim, "idle", false);
            }

            bool charging = pressedKeys.count(Key::Space) > 0;
            ensureChargeOverlay(registry, localId, charging);
            if (charging) {
                updateChargeOverlayPosition(registry, localId);
                auto &anims = registry.get_components<component::animation>();
                if (chargeOverlayLocalId.has_value()) {
                    size_t idx = chargeOverlayLocalId.value();
                    if (idx != localId && idx < anims.size() && anims[idx]) {
                        setAnimation(*anims[idx], "charge", false);
                    }
                }
            }
        }
    }
}

void R_Type::Player::ensureChargeOverlay(engine::registry& registry, size_t playerLocalId, bool show)
{
    if (show && chargeTexture) {
        if (!chargeOverlayLocalId.has_value()) {
            auto e = registry.spawn_entity();
            size_t idx = static_cast<size_t>(e);
            if (idx == playerLocalId) {
                e = registry.spawn_entity();
                idx = static_cast<size_t>(e);
            }
            registry.add_component(e, component::position{0.f, 0.f});
            registry.emplace_component<component::drawable>(e, chargeTexture, chargeRect, layers::Effects);
            registry.add_component(e, component::animation{});
            auto &anims = registry.get_components<component::animation>();
            if (idx < anims.size() && anims[idx]) {
                *anims[idx] = chargeAnimation;
            }
            chargeOverlayLocalId = idx;
        }
    } else {
        if (chargeOverlayLocalId.has_value()) {
            size_t idx = chargeOverlayLocalId.value();
            if (idx != playerLocalId) {
                engine::entity_t e = static_cast<engine::entity_t>(idx);
                registry.kill_entity(e);
            }
            chargeOverlayLocalId.reset();
        }
    }
}

void R_Type::Player::updateChargeOverlayPosition(engine::registry& registry, size_t playerLocalId)
{
    if (!chargeOverlayLocalId.has_value()) return;
    size_t idx = chargeOverlayLocalId.value();
    auto &positions = registry.get_components<component::position>();
    if (playerLocalId < positions.size() && positions[playerLocalId]) {
        auto p = positions[playerLocalId].value();
        float offsetX = 100.f;
        float offsetY = 10.f;
        if (idx != playerLocalId && idx < positions.size() && positions[idx]) {
            positions[idx]->x = p.x + offsetX;
            positions[idx]->y = p.y + offsetY;
        }
    }
}
