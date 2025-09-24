#include <iostream>
#include "Rtype.hpp"
#include "R_Graphic/Vectors.hpp"
#include "R_Graphic/Error.hpp"

R_Type::Rtype::Rtype()
: _app("R-Type", 800, 600)
{
    try
    {
        R_Graphic::intVec2 pos = _app.getWindow().getSize();
        _background = std::make_unique<R_Graphic::Texture>(
            _app.getWindow(),
            "../../Assets/Background/Starfield.png",
            R_Graphic::doubleVec2(0.0, 0.0),
            _app.getWindow().getSize()
        );
        _background_bis = std::make_unique<R_Graphic::Texture>(
            _app.getWindow(),
            "../../Assets/Background/Starfield.png",
            R_Graphic::doubleVec2(
                static_cast<double>(pos.x),
                0.0
            ),
            _app.getWindow().getSize()
        );
    }
    catch(const R_Graphic::Error& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

void R_Type::Rtype::update(float deltaTime)
{
    float move = 100.0 * deltaTime;
    int width = _app.getWindow().getSize().x;

    _background->position.x -= move;
    _background_bis->position.x -= move;
    if (_background->position.x + width <= 0)
        _background->position.x = _background_bis->position.x + width;
    if (_background_bis->position.x + width <= 0)
        _background_bis->position.x = _background->position.x + width;
}

void R_Type::Rtype::draw()
{
    _background->draw(_app.getWindow(), nullptr);
    _background_bis->draw(_app.getWindow(), nullptr);
}

R_Graphic::App& R_Type::Rtype::getApp()
{
    return _app;
}