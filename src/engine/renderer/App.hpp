#pragma once
#include <string>
#include <chrono>
#include <memory>
#include <functional>
#include "Window.hpp"
#include "Renderer.hpp"
#include "engine/events/Events.hpp"

namespace engine {
    namespace R_Graphic
    {
        class App {
            public:
                App(const std::string name, const int width, const int height);
                ~App() = default;
                void run(std::function<void(float,
                    const std::vector<R_Events::Event>&)> gameUpdate,
                    std::function<void()> gameDraw);
                Window& getWindow();
                const std::vector<R_Events::Event>& getEvents() const {
                    return _events;
                }
            private:
                std::vector<R_Events::Event> _events;
                std::unique_ptr<Renderer> _renderer;
                Window _window;
        };
    }
}