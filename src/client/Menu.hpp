/**
 * @file Menu.hpp
 * @brief Definition of the Menu class used in the R-Type engine.
 *
 * The Menu class represents the main game menu in the R-Type project. 
 * It handles the display of the menu interface, including background, 
 * title, and interactive buttons (Start, Quit, Settings). It also 
 * processes user events to update the menu state.
 *
 * @details 
 * - Constructed with a reference to the application renderer (App).
 * - Provides an update method to handle user input events.
 * - Provides a draw method to render the menu interface on screen.
 * - Internally manages background and button textures, as well as the 
 *   title graphics.
 *
 * @namespace R_Type
 * Namespace for the R-Type project.
 *
 * @class R_Type::Menu
 * @brief Class managing the main menu of the R-Type game.
 *
 * @see engine::R_Graphic::App
 * @see engine::R_Graphic::Texture
 * @see engine::R_Events::Event
 */

#pragma once
#include <memory>
#include "engine/renderer/App.hpp"
#include "engine/renderer/Texture.hpp"
#include "engine/events/Events.hpp"

namespace R_Type {
    class Menu {
    public:
        explicit Menu(engine::R_Graphic::App &app);
        ~Menu() = default;

        bool update(const std::vector<engine::R_Events::Event> &events);

        void draw();

    private:
        engine::R_Graphic::App &_app;
        std::shared_ptr<engine::R_Graphic::Texture> _background;
        std::shared_ptr<engine::R_Graphic::Texture> _startButton;
        std::shared_ptr<engine::R_Graphic::Texture> _quitButton;
        std::shared_ptr<engine::R_Graphic::Texture> _settingsButton;
        std::vector<std::shared_ptr<engine::R_Graphic::Texture>> _titleLetters;

        bool _startPressed = false;
        bool _quitPressed = false;
    };
}
