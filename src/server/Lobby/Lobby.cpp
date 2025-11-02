#include <mutex>
#include <iostream>
#include "Lobby.hpp"
#include "engine/profiling/Profiler.hpp"
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
    std::cout << "[Lobby " << _id << "] started\n";
}

void Lobby::stop()
{
    _running = false;
    std::cout << "[Lobby " << _id << "] stopped\n";
}

bool Lobby::has_player(const engine::net::Endpoint &ep) const
{
    return std::any_of(_players.begin(), _players.end(), [&](const auto &p) {
        return p.address == ep.address && p.port == ep.port;
    });
}

void Lobby::add_player(const engine::net::Endpoint &ep)
{
    if (_players.size() >= _maxPlayers)
        return;
    if (has_player(ep))
        return;
    _players.push_back(ep);
    if (_players.size() == _maxPlayers) {
        _ready = true;
        _game.getRunning() = true;
    }
    std::size_t playerIndex = _game.getPlayers().size();
    auto eid = _game.spawn_player(ep, playerIndex);
    PlayerInfo pi{ep, eid};
    _game.getLiveEntities().insert(static_cast<uint32_t>(eid));
    _game.getPlayers().push_back(pi);
    ConnectAck ack{1234, 60, static_cast<uint16_t>(eid)};
    PacketHeader h{CONNECT_ACK, static_cast<uint16_t>(sizeof(ConnectAck)), 0};
    std::vector<uint8_t> buf(sizeof(ConnectAck));
    std::memcpy(buf.data(), &ack, sizeof(ConnectAck));
    _server.send(h, buf, ep);
    _game.broadcast_snapshot();
    std::cout << "[Lobby " << _id << "] Player joined (" << _players.size() << "/" << _maxPlayers << ")\n";
}

void Lobby::remove_player(const engine::net::Endpoint &ep)
{
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
    auto& profiler = Engine::Profiling::Profiler::getInstance();
    uint32_t frameCounter = 0;

    if (_ready) {
        profiler.beginFrame();
        PROFILE_SCOPE("Game Tick");
        {
            PROFILE_SCOPE("Update Game Logic");
            _game.update_game_logic();
        }
        {
            PROFILE_SCOPE("Spawns & Events");
            _game.update_spawns_and_events();
        }
        {
            PROFILE_SCOPE("Broadcast Snapshot");
            _game.broadcast_snapshot();
        }
        _game.getTick()++;
        if (!_game.getRunning()) {
            stop();
        }
        profiler.endFrame();
        if (++frameCounter % 300 == 0) {
            profiler.updateMemoryMetrics();
            profiler.updateCPUMetrics();
            profiler.setEntityCount(_game.getLiveEntities().size());

            const auto& frameMetrics = profiler.getFrameMetrics();
            const auto& memMetrics = profiler.getMemoryMetrics();

            // std::cout << "[Profiling] FPS: " << std::fixed << std::setprecision(1) << frameMetrics.displayFps
            //           << " | Frame: " << frameMetrics.displayFrameTime << "ms"
            //           << " | Entities: " << _live_entities.size()
            //           << " | Memory: " << (memMetrics.physicalMemoryUsed / 1024.0 / 1024.0) << "MB\n";
        }
    }
}
