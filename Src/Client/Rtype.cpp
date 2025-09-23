#include "Rtype.hpp"

R_Type::Rtype::Rtype()
: _app("R-Type", 800, 600)
{
    _background = std::make_unique<R_Graphic::Background>(_app.getWindow().getRenderer(),
        _app.getWindow().getWindow(),
        "../../Assets/Background/Starfield.png", 100.0f);
}

void R_Type::Rtype::update(float deltaTime)
{
    _background->update(deltaTime);
}

void R_Type::Rtype::draw()
{
    _background->draw(_app.getWindow().getRenderer());
}

R_Graphic::App& R_Type::Rtype::getApp()
{
    return _app;
}