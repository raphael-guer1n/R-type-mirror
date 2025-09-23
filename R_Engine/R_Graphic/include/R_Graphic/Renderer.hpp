#pragma once
#include "Window.hpp"

class Renderer {
    public:
        Renderer(Window &window);
        void clear();
        void display();
    private:
        Window &_window;
};
