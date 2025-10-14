
#pragma once
#include <iostream>
#include <SDL.h>
#include "engine/ecs/Registry.hpp"
#include "common/Components.hpp"
#include "common/Components_client_sdl.hpp"
#include "common/Components_client.hpp"
#include "engine/ecs/iterator/Zipper.hpp"
#include "engine/ecs/iterator/Indexed_zipper.hpp"
#include "engine/renderer/App.hpp"
#include "engine/renderer/Window.hpp"
/**
 * @file Systems_client_sdl.hpp
 * @brief Defines ECS systems for client-side SDL rendering and control in the R-Type project.
 *
 * This header provides implementations for several core systems used in the client-side
 * entity-component-system (ECS) architecture, specifically tailored for SDL rendering and
 * input handling. The systems operate on sparse arrays of components and interact with
 * the rendering engine and application window.
 *
 * @details
 * - control_system: Resets the velocity of all controllable entities to zero. This is typically
 *   called at the start of each frame before input is processed, ensuring that velocity is only
 *   set in response to current input.
 *
 * - draw_system: Renders all drawable entities whose position and drawable components are present.
 *   Entities are drawn in order of their layer property, allowing for correct layering of sprites.
 *   The system updates the texture position and invokes the draw method for each entity.
 *
 * - scroll_reset_system: Handles background scrolling for entities marked as 'decor'. When a
 *   background entity moves out of the visible window (to the left), its position is reset to
 *   create a seamless scrolling effect. Requires at least two background entities to function
 *   correctly.
 *
 * @note
 * These systems are designed to be used with the custom ECS framework and rendering engine
 * provided in the R-Type project. They assume the presence of specific component types and
 * engine interfaces.
 *
 * @author marysekatary
 */


using namespace engine;

inline void control_system(registry &r,
    sparse_array<component::velocity> &velocities,
    sparse_array<component::controllable> &controls)
{
    for (auto &&[i, vel, c] : indexed_zipper(velocities, controls))
    {
        vel.vx = 0.f;
        vel.vy = 0.f;
    }
}

inline void draw_system(registry &r,
                        sparse_array<component::position> &positions,
                        sparse_array<component::drawable> &drawables,
                        R_Graphic::Window &window)
{
    std::vector<size_t> indices;
    for (size_t i = 0; i < drawables.size(); i++) {
        if (drawables[i] && positions[i])
            indices.push_back(i);
    }
    std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
        return drawables[a]->layer < drawables[b]->layer;
    });
    auto &hitboxes = r.get_components<component::hitbox>();
    for (size_t idx: indices) {
        auto &d = *drawables[idx];
        auto &p = *positions[idx];
        auto texSize = d.texture->getSize();
        double drawX = p.x;
        double drawY = p.y;
        if (idx < hitboxes.size() && hitboxes[idx]) {
            const auto &hb = hitboxes[idx].value();
            const double cx = static_cast<double>(p.x + hb.offset_x + hb.width * 0.5f);
            const double cy = static_cast<double>(p.y + hb.offset_y + hb.height * 0.5f);
            drawX = cx - static_cast<double>(texSize.x) * 0.5;
            drawY = cy - static_cast<double>(texSize.y) * 0.5;
        }
        d.texture->position = {drawX, drawY};
        d.texture->draw(window, &d.rect);
    }
}

inline void hitbox_overlay_system(registry &r, sparse_array<component::position> &positions,
    sparse_array<component::hitbox> &hitboxes,
    sparse_array<component::entity_kind> &kinds,
    R_Graphic::Window &window,
    int thickness = 2)
{
    SDL_Renderer *ren = window.getRenderer();
    if (!ren) return;
    if (thickness < 1) thickness = 1;

    for (size_t i = 0; i < positions.size(); ++i) {
        if (!(i < positions.size() && positions[i]) ||
            !(i < hitboxes.size() && hitboxes[i]) ||
            !(i < kinds.size() && kinds[i]))
            continue;

        const auto &pos = positions[i].value();
        const auto &hb = hitboxes[i].value();
        const auto kind = kinds[i].value();

        if (kind == component::entity_kind::player) {
            SDL_SetRenderDrawColor(ren, 0, 200, 255, 220);
        } else if (kind == component::entity_kind::enemy) {
            SDL_SetRenderDrawColor(ren, 255, 60, 60, 220);
        } else if (kind == component::entity_kind::projectile) {
            SDL_SetRenderDrawColor(ren, 255, 255, 0, 220);
        } else if (kind == component::entity_kind::projectile_bomb) {
            SDL_SetRenderDrawColor(ren, 255, 180, 0, 220);
        } else if (kind == component::entity_kind::projectile_charged) {
            SDL_SetRenderDrawColor(ren, 255, 120, 0, 220);
        } else if (kind == component::entity_kind::missile_explosion) {
            SDL_SetRenderDrawColor(ren, 0, 255, 120, 200);
        } else {
            SDL_SetRenderDrawColor(ren, 255, 0, 255, 200);
        }

        const float baseX = pos.x + hb.offset_x;
        const float baseY = pos.y + hb.offset_y;
        const float w = hb.width;
        const float h = hb.height;
        if (w <= 0.f || h <= 0.f) continue;

        for (int k = 0; k < thickness; ++k) {
            SDL_Rect rect{ static_cast<int>(baseX) - k,
                           static_cast<int>(baseY) - k,
                           static_cast<int>(w) + 2*k,
                           static_cast<int>(h) + 2*k };
            SDL_RenderDrawRect(ren, &rect);
        }
    }
}

inline void scroll_reset_system(engine::registry &r,
                                engine::sparse_array<component::position> &positions,
                                engine::sparse_array<component::entity_kind> &kinds,
                                R_Graphic::App &app)
{
    int width = app.getWindow().getSize().x;
    std::vector<component::position *> bg_positions;

    for (size_t i = 0; i < kinds.size(); ++i)
    {
        if (kinds[i].has_value() && kinds[i].value() == component::entity_kind::decor)
        {
            if (positions[i].has_value())
            {
                bg_positions.push_back(&positions[i].value());
            }
        }
    }
    if (bg_positions.size() < 2)
    {
        std::cerr << "Error: not enough background entities ("
                  << bg_positions.size() << " found)\n";
        return;
    }
    auto &bg1 = *bg_positions[0];
    auto &bg2 = *bg_positions[1];
    if (bg1.x + width <= 0)
        bg1.x = bg2.x + width;
    if (bg2.x + width <= 0)
        bg2.x = bg1.x + width;
}


