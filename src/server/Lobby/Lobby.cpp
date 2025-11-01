#include <mutex>
#include <iostream>
#include "Lobby.hpp"
#include "server/Server.hpp"

Lobby::Lobby(uint8_t id, std::string name, engine::net::NetServer &server)
    : _id(id), _name(std::move(name)), _server(server), _game(server)
{
}

Lobby::~Lobby()
{
    stop();
}


void Lobby::start()
{
    if (_running)
        return;
    _running = true;
    // _thread = std::thread(&Lobby::run_thread, this);
    std::cout << "[Lobby " << _id << "] started\n";
}

void Lobby::stop()
{
    _running = false;
    // if (_thread.joinable())
    //     _thread.join();
    std::cout << "[Lobby " << _id << "] stopped\n";
}

bool Lobby::has_player(const engine::net::Endpoint &ep) const
{
    std::lock_guard<std::mutex> lock(_playerMtx);
    return std::any_of(_players.begin(), _players.end(), [&](const auto &p) {
        return p.address == ep.address && p.port == ep.port;
    });
}

void Lobby::run_thread()
{
    using namespace std::chrono_literals;
    auto last_tick = std::chrono::steady_clock::now();
    const auto tick_duration = 16ms;

    while (_running)
    {
        auto now = std::chrono::steady_clock::now();
        if (now - last_tick >= tick_duration)
        {
            update();
            last_tick += tick_duration;
        }
        std::this_thread::sleep_for(1ms);
    }
}

void Lobby::add_player(const engine::net::Endpoint &ep)
{
    std::lock_guard<std::mutex> lock(_playerMtx);
    if (_players.size() >= _maxPlayers)
        return;
    if (has_player(ep))
        return;

    _players.push_back(ep);
    PacketHeader hdr{CONNECT_ACK, 0, 0};
    _server.send(hdr, {}, ep);
    std::cout << "[Lobby " << _id << "] Player joined (" << _players.size() << "/" << _maxPlayers << ")\n";
}

void Lobby::remove_player(const engine::net::Endpoint &ep)
{
    std::lock_guard<std::mutex> lock(_playerMtx);
    _players.erase(std::remove_if(_players.begin(), _players.end(),
        [&](const auto &p)
        { return p.address == ep.address && p.port == ep.port; }),
        _players.end()
    );
    std::cout << "[Lobby " << _id << "] Player left (" << _players.size() << " remaining)\n";
}

void Lobby::handle_packet(const engine::net::Endpoint &sender,
    const PacketHeader &hdr,
    const std::vector<uint8_t> &payload)
{
    if (!has_player(sender)) return;

    if (hdr.type == INPUT_PKT) {
        _game.handle_input(sender, payload);
    }
}

void Lobby::update()
{
    std::lock_guard<std::mutex> lock(_playerMtx);

    _game.update_game_logic();
    _game.update_spawns_and_events();
    _game.broadcast_snapshot();
}
