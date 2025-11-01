#pragma once
#include <memory>
#include <unordered_set>
#include "engine/network/IoContext.hpp"
#include "engine/network/UdpSocket.hpp"
#include "engine/network/Endpoint.hpp"
#include "engine/network/NetClient.hpp"
#include "Hud.hpp"
#include "Menu.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "engine/renderer/App.hpp"
#include "engine/events/Events.hpp"
#include "engine/ecs/Registry.hpp"
#include "common/Packets.hpp"
#include "Background.hpp"
#include "Gameover.hpp"

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
        void requestLobbyList();
        void createLobby(const std::string& name);
        void joinLobby(uint8_t lobbyId);
        std::vector<LobbyInfo> getLobbies();

    private:
        void waiting_connection();
        void handle_collision(engine::registry &reg, size_t i, size_t j);
        void handleListLobby(const std::vector<uint8_t> &payload);

    private:
        std::unique_ptr<engine::net::NetClient> _client;
        std::vector<std::pair<PacketHeader, std::vector<uint8_t>>> _pendingSnapshots;
        uint32_t _tick = 0;
        std::unordered_set<engine::R_Events::Key> _pressedKeys;
        uint32_t _player = 0;
        std::unordered_set<uint32_t> _activeEntities;
        engine::R_Graphic::App _app;
        engine::registry _registry;
        std::unique_ptr<Background> _background;
        std::unique_ptr<Player> _playerData;
        std::unique_ptr<Enemy> _enemyData;
        std::unordered_map<uint32_t, size_t> _entityMap;
        std::unique_ptr<Hud> _hud;
        std::vector<float> _hbW, _hbH, _hbOX, _hbOY;
        std::unique_ptr<R_Type::Menu> _menu;
        std::unique_ptr<R_Type::Gameover> _gameOverScreen;
        bool _inMenu = true;
        bool _connected = false;
        bool _gameOver = false;
        bool _won = false;
        std::unordered_map<size_t, int> _playerIndexByLocalId;
        bool _showHitboxes = false;
        int _hitboxOverlayThickness = 3;
        std::vector<LobbyInfo> _lobbies;
    };
    void setAnimation(component::animation &anim, const std::string &clip, bool reverse);
}
