#pragma once
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <random>
#include "engine/ecs/Registry.hpp"
#include "common/Components.hpp"
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

    private:
        void run_thread();

    private:

        uint8_t _id;
        std::string _name;
        uint8_t _maxPlayers = 4;
        std::atomic<bool> _running{false};

        engine::net::NetServer &_server;
        std::vector<engine::net::Endpoint> _players;
        mutable std::mutex _playerMtx;

        std::thread _thread;

        GameLogic _game;
};