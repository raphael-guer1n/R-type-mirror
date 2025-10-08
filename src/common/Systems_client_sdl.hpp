#pragma once
#include <iostream>
#include "engine/ecs/Registry.hpp"
#include "common/Components.hpp"
#include "common/Components_client_sdl.hpp"
#include "engine/ecs/iterator/Zipper.hpp"
#include "engine/ecs/iterator/Indexed_zipper.hpp"
#include "engine/renderer/App.hpp"
#include "engine/renderer/Window.hpp"
#include <SDL_ttf.h>

using namespace engine;

inline void control_system(registry &r,
    sparse_array<component::velocity> &velocities,
    sparse_array<component::controllable> &controls,
    uint8_t keys)
{
    for (auto &&[i, vel, c] : indexed_zipper(velocities, controls))
    {
        vel.vx = 0.f;
        vel.vy = 0.f;
        // if (keys & 0x01) vel.vx = -200.f;
        // if (keys & 0x02) vel.vx =  200.f;
        // if (keys & 0x04) vel.vy = -200.f;
        // if (keys & 0x08) vel.vy =  200.f;
    }
}


inline void draw_system(registry &r,
    sparse_array<component::position> &positions,
    sparse_array<component::drawable> &drawables,
    sparse_array<component::entity_kind> &kinds,
    R_Graphic::Window &window,
    TTF_Font* font)
{
    SDL_Renderer* renderer = window.getRenderer();

    for (auto &&[i, pos, dr, kind] : indexed_zipper(positions, drawables, kinds))
    {
        SDL_Color color;
        switch (kind) {
            case component::entity_kind::player:
                color = {0, 255, 0, 255};
                break;
            case component::entity_kind::enemy:
                color = {255, 0, 0, 255};
                break;
            case component::entity_kind::projectile:
                color = {255, 255, 0, 255};
                break;
            default:
                color = {200, 200, 200, 255};
        }

        SDL_Rect rect = {
            static_cast<int>(pos.x),
            static_cast<int>(pos.y),
            static_cast<int>(dr.rect.size.x),
            static_cast<int>(dr.rect.size.y)
        };

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);

        // --- DEBUG: draw entity index above enemy rectangles ---
        if (kind == component::entity_kind::enemy && font) {
            std::string text = std::to_string(i);
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface* surf = TTF_RenderText_Solid(font, text.c_str(), white);
            if (surf) {
                SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
                if (tex) {
                    SDL_Rect dst {
                        rect.x,
                        rect.y - surf->h - 2,
                        surf->w,
                        surf->h
                    };
                    SDL_RenderCopy(renderer, tex, nullptr, &dst);
                    SDL_DestroyTexture(tex);
                }
                SDL_FreeSurface(surf);
            }
        }
    }
}

inline void scroll_reset_system(engine::registry& r,
    engine::sparse_array<component::position> &positions,
    engine::sparse_array<component::entity_kind> &kinds,
    R_Graphic::App &app)
{
    int width = app.getWindow().getSize().x;
    std::vector<component::position*> bg_positions;

    for (size_t i = 0; i < kinds.size(); ++i) {
        if (kinds[i].has_value() && kinds[i].value() == component::entity_kind::decor) {
            if (positions[i].has_value()) {
                bg_positions.push_back(&positions[i].value());
            }
        }
    }
    if (bg_positions.size() < 2) {
        std::cerr << "Error: not enough background entities ("
            << bg_positions.size() << " found)\n";
        return;
    }
    auto& bg1 = *bg_positions[0];
    auto& bg2 = *bg_positions[1];
    if (bg1.x + width <= 0)
        bg1.x = bg2.x + width;
    if (bg2.x + width <= 0)
        bg2.x = bg1.x + width;
}