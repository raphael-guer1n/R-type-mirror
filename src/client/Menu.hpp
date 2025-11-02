/**
 * @file Menu.hpp
 * @brief Declaration of the Menu class used to manage the R-Type main menu and its submenus.
 *
 * The Menu class handles rendering, interaction, and navigation between the different
 * menu pages (Main, Settings, Help). It also manages button textures, menu music,
 * and accessibility options.
 */

#pragma once

#include <SDL_ttf.h>
#include "engine/renderer/App.hpp"
#include "engine/renderer/Texture.hpp"
#include "engine/events/Events.hpp"
#include "engine/audio/Music.hpp"

namespace R_Type {

/**
 * @class Menu
 * @brief Represents the user interface menu system for the R-Type game.
 *
 * The Menu class is responsible for displaying and managing the game's main menu system.
 * It includes logic for handling user input events, rendering buttons, toggling fullscreen mode,
 * controlling music playback, and providing accessibility options.
 *
 * @note The Menu interacts directly with the rendering engine through @ref engine::R_Graphic::App
 * and uses @ref engine::audio::Music for managing background and gameplay music.
 */
class Menu {
public:
    /**
     * @brief Constructs a new Menu instance.
     *
     * Initializes menu resources and associates the menu with the rendering application.
     *
     * @param app Reference to the main rendering application.
     */
    explicit Menu(engine::R_Graphic::App &app);

    /**
     * @brief Default destructor for the Menu class.
     */
    ~Menu() = default;

    bool update(const std::vector<engine::R_Events::Event> &events, Rtype& rtype);
    bool isAccessibilityEnabled() const { return _accessibilityMode; }
    void draw(Rtype& rtype);
    void drawHelpMenu();

    private:
        enum class Page {
            Main,
            Settings,
            Help,
            Lobby
        };
        engine::audio::Music _menuMusic;
        engine::audio::Music _gameMusic;

        void drawMainMenu();
        void drawSettingsMenu();
        void drawLobbyMenu(Rtype& rtype);
        void drawLobby(Rtype& rtype);
        void drawText(SDL_Renderer* renderer,
            const std::string& text, int x, int y, SDL_Color color);

    /**
     * @brief Music used during gameplay, preloaded from the menu.
     */
    engine::audio::Music _gameMusic;

        std::shared_ptr<engine::R_Graphic::Texture> _background;
        std::shared_ptr<engine::R_Graphic::Texture> _refresh;
        std::shared_ptr<engine::R_Graphic::Texture> _settingsBackground;

        std::shared_ptr<engine::R_Graphic::Texture> _startButton;
        std::shared_ptr<engine::R_Graphic::Texture> _settingsButton;
        std::shared_ptr<engine::R_Graphic::Texture> _accessibilityButton;
        std::shared_ptr<engine::R_Graphic::Texture> _helpButton;
        std::shared_ptr<engine::R_Graphic::Texture> _quitButton;
        std::shared_ptr<engine::R_Graphic::Texture> _acceptButton;

    /**
     * @brief Reference to the rendering application used for drawing.
     */
    engine::R_Graphic::App &_app;

    /**
     * @brief Current active page in the menu.
     */
    Page _currentPage = Page::Main;

        std::shared_ptr<engine::R_Graphic::Texture> _input;
        std::vector<engine::R_Graphic::Texture> _lobbiesBg;

        std::vector<std::shared_ptr<engine::R_Graphic::Texture>> _titleLetters;

    /** @name Button Textures */
    ///@{
    std::shared_ptr<engine::R_Graphic::Texture> _startButton;          /**< Start game button. */
    std::shared_ptr<engine::R_Graphic::Texture> _settingsButton;       /**< Open settings button. */
    std::shared_ptr<engine::R_Graphic::Texture> _accessibilityButton;  /**< Toggle accessibility mode button. */
    std::shared_ptr<engine::R_Graphic::Texture> _helpButton;           /**< Open help menu button. */
    std::shared_ptr<engine::R_Graphic::Texture> _quitButton;           /**< Quit game button. */

    std::shared_ptr<engine::R_Graphic::Texture> _backButton;           /**< Return/back navigation button. */
    std::shared_ptr<engine::R_Graphic::Texture> _soundButton;          /**< Toggle sound button. */
    std::shared_ptr<engine::R_Graphic::Texture> _muteButton;           /**< Mute/unmute button. */
    std::shared_ptr<engine::R_Graphic::Texture> _windowButton;         /**< Toggle fullscreen/windowed mode button. */
    ///@}

    /**
     * @brief Position of the back button on screen.
     */
    engine::R_Graphic::doubleVec2 _backButtonPos;

    /**
     * @brief Vector containing textures for each letter of the game title.
     */
    std::vector<std::shared_ptr<engine::R_Graphic::Texture>> _titleLetters;

    /** @name Menu State Flags */
    ///@{
    bool _startPressed = false;        /**< True if the Start button was pressed. */
    bool _quitPressed = false;         /**< True if the Quit button was pressed. */
    bool _accessibilityMode = false;   /**< Accessibility mode flag. */
    ///@}

    /** @name Settings Flags */
    ///@{
    bool _soundEnabled = true;         /**< Indicates if sound is enabled. */
    bool _fullscreen = false;          /**< Indicates if the game is in fullscreen mode. */
    ///@}

    /** @name UI Layout Parameters */
    ///@{
    int _buttonWidth = 0;              /**< Width of the buttons. */
    int _buttonHeight = 0;             /**< Height of the buttons. */
    int _centerX = 0;                  /**< X coordinate of the screen center. */
    int _winH = 0;                     /**< Height of the window. */
    ///@}
};
}
