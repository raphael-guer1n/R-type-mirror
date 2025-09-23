#pragma once
#include "Window.hpp"

namespace R_Graphic
{
    class Renderer
    {
    public:
        Renderer(Window &window);
        void clear();
        void display();

    private:
        Window &_window;
    };
}
