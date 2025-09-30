#pragma once
#include <string>
#include <chrono>
#include <memory>
#include <functional>
#include "Window.hpp"
#include "Renderer.hpp"

namespace R_Graphic
{
    class App {
        public:
            App(const std::string name, const int width, const int height);
            ~App() = default;
            void run(std::function<void(float)> gameUpdate,
                std::function<void()> gameDraw);
            Window& getWindow();
        private:
            std::unique_ptr<Renderer> _renderer;
            Window _window;
    };
}
