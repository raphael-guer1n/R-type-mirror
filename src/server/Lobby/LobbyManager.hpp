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
        std::atomic<uint8_t> _nextId{1};
};