#include "Menu.hpp"
#include <iostream>
#include <cstdlib> 

R_Type::Menu::Menu(engine::R_Graphic::App &app)
    : _app(app)
{

    _background = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/menu_bg.png",
        engine::R_Graphic::doubleVec2(0, 0),
        engine::R_Graphic::intVec2(1920, 1080)
    );

    _startButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/start_btn.png",
        engine::R_Graphic::doubleVec2(550, 300),
        engine::R_Graphic::intVec2(400, 120)
    );

    _settingsButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/settings_btn.png",
        engine::R_Graphic::doubleVec2(550, 450),
        engine::R_Graphic::intVec2(400, 120)
    );

    _quitButton = std::make_shared<engine::R_Graphic::Texture>(
        _app.getWindow(),
        "./Assets/Menu/quit_btn.png",
        engine::R_Graphic::doubleVec2(550, 600),
        engine::R_Graphic::intVec2(400, 120)
    );

}

bool R_Type::Menu::update(const std::vector<engine::R_Events::Event> &events)
{
    for (auto &ev : events) {
        if (ev.type == engine::R_Events::Type::MouseButtonDown) {
            int x = ev.mouse.x;
            int y = ev.mouse.y;

            if (x >= 550 && x <= 950 && y >= 300 && y <= 420) {
                _startPressed = true;
                return true;
            }
            if (x >= 550 && x <= 950 && y >= 600 && y <= 720) {
                _quitPressed = true;
                std::exit(0);
            }
        }
    }
    draw();
    return false;
}

void R_Type::Menu::draw()
{
    _app.getRenderer().clear();
    _background->draw(_app.getWindow(), nullptr);
    _startButton->draw(_app.getWindow(), nullptr);
    _settingsButton->draw(_app.getWindow(), nullptr);
    _quitButton->draw(_app.getWindow(), nullptr);
    _app.getRenderer().display();
}
