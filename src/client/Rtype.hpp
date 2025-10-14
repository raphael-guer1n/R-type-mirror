#pragma once
#include <memory>
#include <unordered_set>
#include "engine/network/IoContext.hpp"
#include "engine/network/UdpSocket.hpp"
#include "engine/network/Endpoint.hpp"
#include "Hud.hpp"
#include "Menu.hpp"
#include "Player.hpp"
#include "engine/renderer/App.hpp"
#include "engine/events/Events.hpp"
#include "engine/ecs/Registry.hpp"
#include "Background.hpp"

/**
 * @namespace R_Type
 * @brief Namespace containing the main game logic and utilities for the R-Type client.
 */

/**
 * @class Rtype
 * @brief Main class for managing the R-Type client game loop, networking, and rendering.
 *
 * The Rtype class encapsulates the core functionalities of the R-Type client, including:
 * - Handling network communication with the server via UDP.
 * - Managing game state updates, entity registry, and player data.
 * - Processing input events and maintaining active entities.
 * - Rendering graphics and background.
 * - Providing access to the application window and registry.
 *
 * @note This class is not copyable.
 */

/**
 * @brief Updates the game state based on elapsed time and input events.
 * @param deltaTime Time elapsed since the last update (in seconds).
 * @param events List of input or system events to process.
 */

/**
 * @brief Receives and processes a snapshot from the server to synchronize game state.
 */

/**
 * @brief Renders the current game state to the application window.
 */

/**
 * @brief Returns a reference to the application window.
 * @return Reference to engine::R_Graphic::App.
 */

/**
 * @brief Returns a reference to the entity registry.
 * @return Reference to engine::registry.
 */

/**
 * @brief Handles the waiting state during connection to the server.
 */

/**
 * @brief Sets the animation clip and direction for a given animation component.
 * @param anim Reference to the animation component to modify.
 * @param clip Name of the animation clip to set.
 * @param reverse If true, plays the animation in reverse.
 */
namespace R_Type
{
    class Hud;
    class Rtype
    {
    public:
        Rtype();
        ~Rtype() = default;
        void update(float deltaTime, const std::vector<engine::R_Events::Event> &events);
        void receiveSnapshot();
        void draw();
        engine::R_Graphic::App &getApp();
        engine::registry &getRegistry();

    public:
        void setServerEndpoint(const std::string &ip, unsigned short port);
        void waiting_connection();

    private:
        std::unique_ptr<engine::net::Endpoint> _serverEndpoint;
        uint32_t _tick = 0;
        std::unordered_set<engine::R_Events::Key> _pressedKeys;
        uint32_t _player = 0;
        std::unordered_set<uint32_t> _activeEntities;
        engine::net::Endpoint _sender;
        engine::R_Graphic::App _app;
        engine::registry _registry;
        std::unique_ptr<Background> _background;
        engine::net::IoContext _ioContext;
        std::unique_ptr<engine::net::UdpSocket> _client;
        std::unique_ptr<Player> _playerData;
        std::unordered_map<uint32_t, size_t> _entityMap;
        std::unique_ptr<Hud> _hud;
        std::unique_ptr<R_Type::Menu> _menu;
        bool _inMenu = true;
    };
    void setAnimation(component::animation &anim, const std::string &clip, bool reverse);
}
