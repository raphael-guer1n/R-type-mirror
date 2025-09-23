#pragma once
#include <memory>
#include <functional>
#include "R_Graphic/Background.hpp"
#include "R_Graphic/App.hpp"

namespace R_Type
{
    class Rtype
    {
        public:
            Rtype();
            ~Rtype() = default;
            void update(float deltaTime);
            void draw();
            R_Graphic::App& getApp();
        private:
            R_Graphic::App _app;
            std::unique_ptr<R_Graphic::Background> _background;
    };
}
