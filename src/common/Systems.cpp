#include "engine/ecs/Systems.hpp"
#include <algorithm> // for std::max

namespace engine
{
    void position_system(registry &r,
                         sparse_array<component::position> &positions,
                         sparse_array<component::velocity> &velocities)
    {
        for (auto &&[i, pos, vel] : indexed_zipper(positions, velocities))
        {
            pos.x += vel.vx;
            pos.y += vel.vy;
        }
    }

    void health_system(registry &r,
                       sparse_array<component::health> &healths,
                       sparse_array<component::despawn_tag> &despawns,
                       sparse_array<component::damage> &damages)
    {
        for (auto &&[i, h] : indexed_zipper(healths))
        {
            if (i < damages.size() && damages[i] && damages[i].value().amount != 0)
            {
                int hp = static_cast<int>(h.hp);
                hp -= damages[i].value().amount;
                h.hp = static_cast<std::uint8_t>(std::max(0, hp));
                damages[i].value().amount = 0;
            }
            if (h.hp == 0)
            {
                if (i >= despawns.size())
                    despawns.emplace_at(i, component::despawn_tag{true});
                else if (!despawns[i])
                    despawns.emplace_at(i, component::despawn_tag{true});
                else
                    despawns[i].value().now = true;
            }
        }
    }

    void despawn_system(registry &r,
                        sparse_array<component::despawn_tag> &despawns)
    {
        for (auto &&[i, d] : indexed_zipper(despawns))
        {
            if (d.now)
                r.kill_entity(r.entity_from_index(i));
        }
    }

    void spawn_system(registry &r,
                      sparse_array<component::spawn_request> &spawns)
    {
        for (auto &&[i, req] : indexed_zipper(spawns))
        {
            auto copy = req;          // copy because we clear
            spawns[i] = std::nullopt; // consume request
            auto e = r.spawn_entity();
            if (copy.factory)
                copy.factory(r, e);
        }
    }

    void input_system(registry &,
                      sparse_array<component::controllable> &controls)
    {
        const Uint8 *state = SDL_GetKeyboardState(nullptr);

        for (auto &&[c] : zipper(controls))
        {
            c.inputX = 0;
            c.inputY = 0;

            if (state[SDL_SCANCODE_LEFT])
                c.inputX = -1;
            if (state[SDL_SCANCODE_RIGHT])
                c.inputX = 1;
            if (state[SDL_SCANCODE_UP])
                c.inputY = -1;
            if (state[SDL_SCANCODE_DOWN])
                c.inputY = 1;

            c.shoot = state[SDL_SCANCODE_SPACE];
        }
    }

    void boundary_system(registry &,
                         sparse_array<component::position> &positions,
                         sparse_array<component::drawable> &drawables,
                         sparse_array<component::player_tag> &players,
                         R_Graphic::Window &window)
    {
        int winW = window.getSize().x;
        int winH = window.getSize().y;

        for (size_t i = 0; i < positions.size(); ++i)
        {
            if (!positions[i].has_value() || !drawables[i].has_value() || !players[i].has_value())
                continue;

            auto &pos = positions[i].value();
            auto &dr = drawables[i].value();
            int spriteW = dr.rect.size.x;
            int spriteH = dr.rect.size.y;

            if (pos.x < 0)
                pos.x = 0;
            if (pos.y < 0)
                pos.y = 0;
            if (pos.x + spriteW > winW)
                pos.x = winW - spriteW;
            if (pos.y + spriteH > winH)
                pos.y = winH - spriteH;
        }
    }

    
}