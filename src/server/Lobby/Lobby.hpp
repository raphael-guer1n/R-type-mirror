/**
 * @class Lobby
 * @brief Represents a single multiplayer game lobby.
 *
 * The Lobby class manages a group of players waiting to play a game together.
 * It handles player addition/removal, packet routing, game logic updates, and
 * starting/stopping the game session.
 *
 * ### Responsibilities
 * 1. **Player Management**
 *    - `add_player` / `remove_player` to manage players in the lobby.
 *    - `has_player` checks if a player is part of the lobby.
 *    - `_players` vector stores active player endpoints.
 *    - `_playerMtx` ensures thread-safe access to player data.
 *
 * 2. **Game Lifecycle**
 *    - `start` begins the game, launching the internal game logic.
 *    - `stop` stops the game and ends the lobby session.
 *    - `_running` atomic flag tracks whether the game is active.
 *    - `_ready` flag indicates if the lobby is ready to start the game.
 *
 * 3. **Networking**
 *    - `handle_packet` processes incoming network packets for this lobby.
 *    - Uses `_server` (engine::net::NetServer) to communicate with clients.
 *
 * 4. **Game Logic**
 *    - `_game` manages the actual game ECS, systems, and entity updates.
 *    - `update` is called each server tick to progress the game state.
 *
 * ### Members
 * - `_id`: Unique lobby identifier.
 * - `_name`: Human-readable lobby name.
 * - `_maxPlayers`: Maximum number of players allowed in the lobby.
 * - `_players`: Vector of connected player endpoints.
 * - `_playerMtx`: Mutex protecting `_players`.
 * - `_server`: Reference to network server for sending/receiving packets.
 * - `_thread`: Optional thread for asynchronous operations (if used).
 * - `_game`: Instance of `GameLogic` for running the actual game simulation.
 *
 * ### Notes
 * - This class is not copyable.
 * - Thread-safety is important when multiple players join/leave or send packets concurrently.
 * - Designed for integration with `LobbyManager` to manage multiple lobbies.
 *
 * @see GameLogic
 * @see LobbyManager
 * @see engine::net::NetServer
 */
#pragma once
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <random>
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/Components.hpp"
#include "common/Packets.hpp"
#include "engine/network/IoContext.hpp"
#include "engine/network/UdpSocket.hpp"
#include "engine/network/Endpoint.hpp"
#include "engine/network/NetServer.hpp"
#include "common/Packets.hpp"
#include "engine/network/Endpoint.hpp"
#include "server/GameLogic.hpp"

class Lobby
{
    public:
        Lobby(uint8_t id, std::string name,
            engine::net::NetServer &server);
        ~Lobby();

        void start();
        void stop();

        void add_player(const engine::net::Endpoint &ep);
        void remove_player(const engine::net::Endpoint &ep);
        bool has_player(const engine::net::Endpoint &ep) const;


        void handle_packet(const engine::net::Endpoint &sender,
            const PacketHeader &hdr,
            const std::vector<uint8_t> &payload);

        void update();

        uint8_t id() const { return _id; }
        const std::string &name() const { return _name; }
        uint8_t playerCount() const { return static_cast<uint8_t>(_players.size()); }
        uint8_t maxPlayers() const { return _maxPlayers; }
        bool getRunning() const {return _running;}

    private:

        uint8_t _id;
        std::string _name;
        uint8_t _maxPlayers = MAX_PLAYERS;
        std::atomic<bool> _running{false};
        bool _ready = false;

        engine::net::NetServer &_server;
        std::vector<engine::net::Endpoint> _players;
        mutable std::mutex _playerMtx;

        std::thread _thread;

        GameLogic _game;
};