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

        // Comprehensive engine key enumeration mapped to SDL_Keycode values
        enum class Key : int
        {
            Unknown = SDLK_UNKNOWN,

            // Letters
            A = SDLK_a,
            B = SDLK_b,
            C = SDLK_c,
            D = SDLK_d,
            E = SDLK_e,
            F = SDLK_f,
            G = SDLK_g,
            H = SDLK_h,
            I = SDLK_i,
            J = SDLK_j,
            K = SDLK_k,
            L = SDLK_l,
            M = SDLK_m,
            N = SDLK_n,
            O = SDLK_o,
            P = SDLK_p,
            Q = SDLK_q,
            R = SDLK_r,
            S = SDLK_s,
            T = SDLK_t,
            U = SDLK_u,
            V = SDLK_v,
            W = SDLK_w,
            X = SDLK_x,
            Y = SDLK_y,
            Z = SDLK_z,

            // Digits (top row)
            Num0 = SDLK_0,
            Num1 = SDLK_1,
            Num2 = SDLK_2,
            Num3 = SDLK_3,
            Num4 = SDLK_4,
            Num5 = SDLK_5,
            Num6 = SDLK_6,
            Num7 = SDLK_7,
            Num8 = SDLK_8,
            Num9 = SDLK_9,
            // Arrows

            Left = SDLK_LEFT,
            Right = SDLK_RIGHT,
            Up = SDLK_UP,
            Down = SDLK_DOWN,

            // Whitespace/control
            Space = SDLK_SPACE,
            Enter = SDLK_RETURN,
            Escape = SDLK_ESCAPE,
            Tab = SDLK_TAB,
            Backspace = SDLK_BACKSPACE,

            // Function keys
            F1 = SDLK_F1,
            F2 = SDLK_F2,
            F3 = SDLK_F3,
            F4 = SDLK_F4,
            F5 = SDLK_F5,
            F6 = SDLK_F6,
            F7 = SDLK_F7,
            F8 = SDLK_F8,
            F9 = SDLK_F9,
            F10 = SDLK_F10,
            F11 = SDLK_F11,
            F12 = SDLK_F12,
            F13 = SDLK_F13,
            F14 = SDLK_F14,
            F15 = SDLK_F15,
            F16 = SDLK_F16,
            F17 = SDLK_F17,
            F18 = SDLK_F18,
            F19 = SDLK_F19,
            F20 = SDLK_F20,
            F21 = SDLK_F21,
            F22 = SDLK_F22,
            F23 = SDLK_F23,
            F24 = SDLK_F24,

            // Navigation
            Insert = SDLK_INSERT,
            Delete = SDLK_DELETE,
            Home = SDLK_HOME,
            End = SDLK_END,
            PageUp = SDLK_PAGEUP,
            PageDown = SDLK_PAGEDOWN,

            // Modifiers
            LShift = SDLK_LSHIFT,
            RShift = SDLK_RSHIFT,
            LCtrl = SDLK_LCTRL,
            RCtrl = SDLK_RCTRL,
            LAlt = SDLK_LALT,
            RAlt = SDLK_RALT,
            LGUI = SDLK_LGUI,
            RGUI = SDLK_RGUI,

            // Punctuation / symbols
            Comma = SDLK_COMMA,
            Period = SDLK_PERIOD,
            Slash = SDLK_SLASH,
            Semicolon = SDLK_SEMICOLON,
            Quote = SDLK_QUOTE,
            LeftBracket = SDLK_LEFTBRACKET,
            RightBracket = SDLK_RIGHTBRACKET,
            Backslash = SDLK_BACKSLASH,
            Backquote = SDLK_BACKQUOTE,
            Minus = SDLK_MINUS,
            Equals = SDLK_EQUALS,

            // Keypad
            KP_0 = SDLK_KP_0,
            KP_1 = SDLK_KP_1,
            KP_2 = SDLK_KP_2,
            KP_3 = SDLK_KP_3,
            KP_4 = SDLK_KP_4,
            KP_5 = SDLK_KP_5,
            KP_6 = SDLK_KP_6,
            KP_7 = SDLK_KP_7,
            KP_8 = SDLK_KP_8,
            KP_9 = SDLK_KP_9,
            KP_Plus = SDLK_KP_PLUS,
            KP_Minus = SDLK_KP_MINUS,
            KP_Multiply = SDLK_KP_MULTIPLY,
            KP_Divide = SDLK_KP_DIVIDE,
            KP_Enter = SDLK_KP_ENTER,
            KP_Decimal = SDLK_KP_DECIMAL,

            // Locks / misc
            CapsLock = SDLK_CAPSLOCK,
            NumLockClear = SDLK_NUMLOCKCLEAR,
            ScrollLock = SDLK_SCROLLLOCK,
            PrintScreen = SDLK_PRINTSCREEN,
            Pause = SDLK_PAUSE
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


        static inline engine::R_Events::Key mapSDLKey(SDL_Keycode sym)
        {
            return static_cast<engine::R_Events::Key>(sym);
        }

        inline bool hasEvent(const std::vector<R_Events::Event> &events, Type type) {
            return std::any_of(events.begin(), events.end(),
                [type](const R_Events::Event &e) {
                    return e.type == type;
            });
        }
    }
}
