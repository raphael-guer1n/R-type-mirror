#pragma once
#include "Window.hpp"
#include <vector>
/**
 * @file Renderer.hpp
 * @brief Defines the Renderer class responsible for managing rendering operations.
 *
 * The Renderer class provides an interface to clear and display the contents
 * of a rendering window. It acts as a bridge between the application and the
 * underlying graphics system, encapsulating rendering logic and window management.
 */
namespace engine {
    namespace R_Graphic
    {
        class Renderer
        {
            public:
                Renderer(Window &window);
                void clear();
                void display();

                // Drawing helpers to avoid direct SDL usage in app code
                void setDrawColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
                void fillRect(int x, int y, int w, int h);
                void drawRect(int x, int y, int w, int h);

            private:
                Window &_window;
        };
    }
}