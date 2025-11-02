/**
 * @file Lobby.cpp
 * @brief Handles client-side lobby networking and state management for R-Type.
 *
 * This file implements the `R_Type::Rtype` class functions responsible for interacting with the
 * game server to manage multiplayer lobbies. The class provides methods to request a list of
 * available lobbies, create a new lobby, join an existing lobby, and handle server responses 
 * to update the client’s internal state. Internally, the class maintains a vector of `LobbyInfo`
 * structures (`_lobbies`) representing all known lobbies, tracks whether the client is currently
 * inside a lobby (`_inLobby`), and stores the ID of the current lobby (`_currentLobbyId`). 
 * All network communication is performed through the `_client` object using structured packets 
 * such as `LIST_LOBBIES`, `CREATE_LOBBY`, and `JOIN_LOBBY`. Server responses, including 
 * `LobbyListResponse` and `LobbyJoinedResponse`, are parsed and used to synchronize the client 
 * state with the server. Safety checks are performed, such as ensuring lobby names are under 
 * 32 characters and preventing joining multiple lobbies simultaneously.
 *
 * ### Methods
 * - `requestLobbyList()` – Sends a request to the server for the list of available lobbies.
 * - `createLobby(const std::string &name)` – Sends a request to create a new lobby with the given name.
 * - `joinLobby(uint8_t lobbyId)` – Requests to join a specific lobby by its ID.
 * - `getLobbies()` – Returns the locally cached list of known lobbies.
 * - `handleListLobby(const std::vector<uint8_t> &payload)` – Processes the server response with the lobby list.
 * - `handleLobbyJoined(const std::vector<uint8_t> &payload)` – Processes the server response confirming a lobby join.
 *
 * @note All methods that send network packets construct a `PacketHeader` and a payload buffer 
 * before sending via `_client->send()`. Incoming payloads are copied into structured data types 
 * using `std::memcpy` and then applied to update the client’s lobby state.
 */

#include <iostream>
#include "Rtype.hpp"

void R_Type::Rtype::requestLobbyList()
{
    PacketHeader hdr{LIST_LOBBIES, 0, 0};
    std::vector<uint8_t> empty;
    _client->send(hdr, empty);
}

void R_Type::Rtype::createLobby(const std::string &name)
{
    if (name.size() >= 32 || _inLobby)
        return;
    LobbyCreateRequest req{};
    std::strncpy(req.name, name.c_str(), sizeof(req.name) - 1);
    PacketHeader hdr{CREATE_LOBBY, sizeof(req), 0};
    std::vector<uint8_t> buf(sizeof(req));
    std::memcpy(buf.data(), &req, sizeof(req));
    _client->send(hdr, buf);
}

void R_Type::Rtype::joinLobby(uint8_t lobbyId)
{
    LobbyJoinRequest req{lobbyId};
    PacketHeader hdr{JOIN_LOBBY, sizeof(req), 0};
    std::vector<uint8_t> buf(sizeof(req));
    std::memcpy(buf.data(), &req, sizeof(req));
    _client->send(hdr, buf);
}

std::vector<LobbyInfo> R_Type::Rtype::getLobbies()
{
    return _lobbies;
}

void R_Type::Rtype::handleListLobby(const std::vector<uint8_t> &payload)
{
    LobbyListResponse resp;
    std::memcpy(&resp, payload.data(), sizeof(resp));

    _lobbies.clear();
    for (int i = 0; i < resp.count; ++i)
    {
        LobbyInfo info{};
        info.id = resp.lobbies[i].id;
        info.playerCount = resp.lobbies[i].playerCount;
        info.maxPlayers = resp.lobbies[i].maxPlayers;
        std::strncpy(info.name, resp.lobbies[i].name, sizeof(info.name) - 1);
        info.name[sizeof(info.name) - 1] = '\0';
        _lobbies.push_back(info);
    }
}

void R_Type::Rtype::handleLobbyJoined(const std::vector<uint8_t> &payload)
{
    if (payload.size() < sizeof(LobbyJoinedResponse))
        return;

    LobbyJoinedResponse resp{};
    std::memcpy(&resp, payload.data(), sizeof(resp));

    _inLobby = true;
    _currentLobbyId = resp.lobbyId;
}
