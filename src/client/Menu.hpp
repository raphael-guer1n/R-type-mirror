/**
 * @file Menu.hpp
 * @brief Declaration of the Menu class managing the main game menu in R-Type.
 *
 * The **Menu** class handles the graphical and interactive components
 * of the R-Type main menu, including background, title letters, and
 * interactive buttons such as *Start*, *Settings*, and *Quit*.
 * It also manages background music playback and transitions between
 * menu and in-game audio.
 *
 * @details 
 * The class:
 * - Initializes and displays all visual elements of the main menu.
 * - Processes user events such as mouse clicks to navigate options.
 * - Controls background music:
 *    - Plays `Menu.ogg` when the menu is active.
 *    - Stops it and starts `Game.ogg` when the player launches the game.
 * - Uses the R-Type rendering engine for texture and event handling.
 *
 * ### Example usage:
 * @code
 * engine::R_Graphic::App app("R-Type", 1920, 1080);
 * R_Type::Menu menu(app);
 *
 * while (true) {
 *     auto events = app.getWindow().pollEvents();
 *     if (menu.update(events))
 *         break; // Start button pressed
 *     menu.draw();
 * }
 * @endcode
 *
 * @namespace R_Type
 * Namespace grouping all components of the R-Type game.
 *
 * @class R_Type::Menu
 * @brief Class managing the display, input, and music of the main menu.
 *
 * @see engine::R_Graphic::App
 * @see engine::R_Graphic::Texture
 * @see engine::R_Events::Event
 * @see engine::audio::Music
 */

#pragma once
#include <memory>
#include "engine/renderer/App.hpp"
#include "engine/renderer/Texture.hpp"
#include "engine/events/Events.hpp"
#include "engine/audio/Music.hpp"

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
        engine::audio::Music _menuMusic;
        engine::audio::Music _gameMusic;

        bool _startPressed = false;
        bool _quitPressed = false;
    };
}
