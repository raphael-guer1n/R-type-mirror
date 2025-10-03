#pragma once
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

        struct Event {
            Type type;
            union {
                struct {
                    int key;
                } key;
                struct {
                    int x, y;
                    int button;
                } mouse;
            };
        };
        inline bool hasEvent(const std::vector<R_Events::Event> &events, Type type) {
            return std::any_of(events.begin(), events.end(),
                [type](const R_Events::Event &e) {
                    return e.type == type;
            });
        }
    }
}
