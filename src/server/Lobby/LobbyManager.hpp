/**
 * @class LobbyManager
 * @brief Manages multiple game lobbies and routes network packets for each lobby.
 *
 * The LobbyManager class is responsible for:
 * - Tracking active lobbies and their players.
 * - Handling lobby creation, joining, leaving, and shutdown.
 * - Routing incoming network packets to the appropriate lobby.
 * - Sending lobby lists to clients upon request.
 *
 * ### Responsibilities
 * 1. **Lobby Tracking**
 *    - Maintains `_lobbies`, a map of lobby IDs to `Lobby` instances.
 *    - Tracks which player belongs to which lobby using `_playerToLobby`.
 *
 * 2. **Packet Routing**
 *    - Receives packets from the network (`on_packet`) and forwards them to
 *      the correct lobby or handles lobby management commands.
 *    - Uses `route_to_lobby` internally to delegate packets to specific lobbies.
 *
 * 3. **Lobby Lifecycle**
 *    - `create_lobby`: Creates a new lobby for a player.
 *    - `join_lobby`: Adds a player to an existing lobby.
 *    - `leave_lobby`: Removes a player from a lobby.
 *    - `tick_all`: Calls the `update()` function for all active lobbies each server tick.
 *    - `shutdown`: Cleans up all lobbies when the server stops.
 *
 * ### Thread Safety
 * - `_mtx` protects `_lobbies` and `_playerToLobby` for concurrent access.
 * - Network callbacks may occur in separate threads, requiring careful locking.
 *
 * ### Members
 * - `_mtx`: Mutex for thread-safe lobby management.
 * - `_lobbies`: Map of lobby ID → Lobby shared pointers.
 * - `_playerToLobby`: Map of player identifier → lobby ID.
 * - `_netServer`: Reference to the network server instance.
 * - `_nextId`: ID generator for new lobbies.
 *
 * @note LobbyManager does not copy or move; all lobby pointers are managed internally via `shared_ptr`.
 *
 * @see Lobby
 * @see engine::net::NetServer
 */
#pragma once
#include <memory>
#include <mutex>
#include <atomic>
#include "engine/network/Endpoint.hpp"
#include "common/Packets.hpp"
#include "Lobby.hpp"

class LobbyManager
{
    public:
        explicit LobbyManager(engine::net::NetServer &server);
        ~LobbyManager();

        void tick_all();

        void on_packet(const engine::net::Endpoint &sender,
            const PacketHeader &hdr,
            const std::vector<uint8_t> &payload);

        void send_lobby_list(const engine::net::Endpoint &sender);
        void create_lobby(const engine::net::Endpoint &sender,
            const std::vector<uint8_t> &payload);
        void join_lobby(const engine::net::Endpoint &sender,
            const std::vector<uint8_t> &payload);

        void leave_lobby(const engine::net::Endpoint &sender);

        void shutdown();
    private:
        void route_to_lobby(const engine::net::Endpoint &sender,
            const PacketHeader &hdr,
            const std::vector<uint8_t> &payload);

    private:
        std::mutex _mtx;
        std::unordered_map<uint8_t, std::shared_ptr<Lobby>> _lobbies;
        std::unordered_map<std::string, uint8_t> _playerToLobby;
        engine::net::NetServer &_netServer;
        uint8_t _nextId{1};
};