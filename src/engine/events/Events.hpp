#pragma once
#include <SDL.h>
#include <vector>
#include <algorithm>

namespace engine
{
    namespace R_Events
    {
        enum class Type {
            Quit,
            KeyDown,
            KeyUp,
            MouseButtonDown,
            MouseButtonUp,
            MouseMove,
            FocusGained,
            FocusLost
        };

        enum class Key {
            Unknown = 0,
            Left, Right, Up, Down,
            Z, Q, S, D,
            Space
        };

        struct Event {
            Type type;
            union {
                struct {
                    Key code;
                } key;
                struct {
                    int x, y;
                    int button;
                } mouse;
            };
        };

        //TODO ADD ALL KEYS
        static inline engine::R_Events::Key mapSDLKey(SDL_Keycode sym)
        {
            using namespace engine::R_Events;
            switch (sym) {
                case SDLK_LEFT: return Key::Left;
                case SDLK_RIGHT: return Key::Right;
                case SDLK_UP: return Key::Up;
                case SDLK_DOWN: return Key::Down;
                case SDLK_z: return Key::Z;
                case SDLK_q: return Key::Q;
                case SDLK_s: return Key::S;
                case SDLK_d: return Key::D;
                case SDLK_SPACE: return Key::Space;
                default: return Key::Unknown;
            }
        }

        inline bool hasEvent(const std::vector<R_Events::Event> &events, Type type) {
            return std::any_of(events.begin(), events.end(),
                [type](const R_Events::Event &e) {
                    return e.type == type;
            });
        }
    }
}
