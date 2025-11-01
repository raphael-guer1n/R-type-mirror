#include "Rtype.hpp"

void R_Type::Rtype::requestLobbyList()
{
    PacketHeader hdr{LIST_LOBBIES, 0, 0};
    std::vector<uint8_t> empty;
    _client->send(hdr, empty);
}

void R_Type::Rtype::createLobby(const std::string &name)
{
    if (name.size() >= 32)
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
