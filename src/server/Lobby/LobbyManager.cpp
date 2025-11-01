#include <iostream>
#include "LobbyManager.hpp"

static std::string endpoint_key(const engine::net::Endpoint &ep)
{
    return ep.address + ":" + std::to_string(ep.port);
}

LobbyManager::LobbyManager(engine::net::NetServer &server)
    : _netServer(server)
{
    _netServer.set_packet_handler(
        [this](const engine::net::Endpoint &sender,
            const PacketHeader &hdr,
            const std::vector<uint8_t> &payload)
        {
            this->on_packet(sender, hdr, payload);
        }
    );
}

LobbyManager::~LobbyManager()
{
    shutdown();
}

void LobbyManager::tick_all()
{
    std::lock_guard<std::mutex> lock(_mtx);
    for (auto &[id, lobby] : _lobbies)
        lobby->update();
}

void LobbyManager::shutdown()
{
    std::lock_guard<std::mutex> lock(_mtx);
    for (auto &[id, lobby] : _lobbies)
        lobby->stop();
    _lobbies.clear();
    _playerToLobby.clear();
}

void LobbyManager::on_packet(const engine::net::Endpoint &sender,
    const PacketHeader &hdr,
    const std::vector<uint8_t> &payload)
{
    switch (hdr.type)
    {
        case LIST_LOBBIES:
            std::cout << "List lobby" << std::endl;
            send_lobby_list(sender);
            break;
        case CREATE_LOBBY:
            std::cout << "Create lobby" << std::endl;
            create_lobby(sender, payload);
            break;
        case JOIN_LOBBY:
            join_lobby(sender, payload);
            break;
        default:
            route_to_lobby(sender, hdr, payload);
            break;
    }
}

void LobbyManager::send_lobby_list(const engine::net::Endpoint &sender)
{
    LobbyListResponse resp{};
    std::lock_guard<std::mutex> lock(_mtx);
    uint8_t count = 0;
    for (auto &[id, lobby] : _lobbies)
    {
        if (count >= 16)
            break;
        auto &info = resp.lobbies[count++];
        info.id = id;
        info.playerCount = lobby->playerCount();
        info.maxPlayers = lobby->maxPlayers();
        std::strncpy(info.name, lobby->name().c_str(), sizeof(info.name) - 1);
    }
    resp.count = count;

    PacketHeader hdr{LOBBY_LIST_RESPONSE, sizeof(resp), 0};
    std::vector<uint8_t> buf(sizeof(resp));
    std::memcpy(buf.data(), &resp, sizeof(resp));
    _netServer.send(hdr, buf, sender);
}

void LobbyManager::create_lobby(const engine::net::Endpoint &sender,
    const std::vector<uint8_t> &payload)
{
    if (payload.size() < sizeof(LobbyCreateRequest))
        return;

    LobbyCreateRequest req{};
    std::memcpy(&req, payload.data(), sizeof(req));

    std::string lobbyName(req.name);
    if (lobbyName.empty())
        lobbyName = "Lobby " + std::to_string(_nextId);

    uint8_t id = _nextId++;

    auto lobby = std::make_shared<Lobby>(id, lobbyName, _netServer);
    lobby->start();
    lobby->add_player(sender);

    _lobbies[id] = lobby;
    _playerToLobby[endpoint_key(sender)] = id;

    std::cout << "[LobbyManager] Created lobby '" << lobbyName
        << "' (" << (int)id << ") by " << sender.address << ":" << sender.port << "\n";
    send_lobby_list(sender);
}


void LobbyManager::join_lobby(const engine::net::Endpoint &sender,
    const std::vector<uint8_t> &payload)
{
    if (payload.size() < sizeof(LobbyJoinRequest))
        return;
    LobbyJoinRequest req{};
    std::memcpy(&req, payload.data(), sizeof(req));

    std::lock_guard<std::mutex> lock(_mtx);
    auto it = _lobbies.find(req.lobbyId);
    if (it != _lobbies.end()) {
        it->second->add_player(sender);
        _playerToLobby[endpoint_key(sender)] = req.lobbyId;
    }
}

void LobbyManager::leave_lobby(const engine::net::Endpoint &sender)
{
    std::lock_guard<std::mutex> lock(_mtx);
    std::string key = endpoint_key(sender);
    auto it = _playerToLobby.find(key);
    if (it == _playerToLobby.end())
        return;

    uint8_t lobbyId = it->second;
    auto lit = _lobbies.find(lobbyId);
    if (lit != _lobbies.end())
    {
        lit->second->remove_player(sender);
        if (lit->second->playerCount() == 0)
        {
            lit->second->stop();
            _lobbies.erase(lit);
            std::cout << "[LobbyManager] Removed empty lobby " << lobbyId << "\n";
        }
    }
    _playerToLobby.erase(it);
}

void LobbyManager::route_to_lobby(const engine::net::Endpoint &sender,
    const PacketHeader &hdr,
    const std::vector<uint8_t> &payload)
{
    std::lock_guard<std::mutex> lock(_mtx);
    std::string key = endpoint_key(sender);
    auto it = _playerToLobby.find(key);
    if (it == _playerToLobby.end())
        return;
    uint8_t lobbyId = it->second;
    auto lit = _lobbies.find(lobbyId);
    if (lit != _lobbies.end())
        lit->second->handle_packet(sender, hdr, payload);
}
