#include <iostream>
#include "Server.hpp"

void server::handle_connect(const engine::net::Endpoint &sender)
{
  if (_players.size() >= MAX_PLAYERS) return;

  std::size_t playerIndex = _players.size();
  auto eid = spawn_player(sender, playerIndex);
  PlayerInfo pi{sender, eid};
  _live_entities.insert(static_cast<uint32_t>(eid));
  std::cout << "Spawned player entity: " << eid << " for " << sender.address << ":" << sender.port
    << std::endl;
  _players.push_back(pi);
  ConnectAck ack{1234, 60, static_cast<uint16_t>(eid)};
  PacketHeader h{CONNECT_ACK, static_cast<uint16_t>(sizeof(ConnectAck)), 0};
  std::vector<uint8_t> buf(sizeof(ConnectAck));
  std::memcpy(buf.data(), &ack, sizeof(ConnectAck));
  _netServer.send(h, buf, sender);
  broadcast_snapshot();

  std::cout << "Player " << eid << " connected from " << sender.address << ":" << sender.port << std::endl;

  if (_players.size() == MAX_PLAYERS)
  {
    _ready = true;
    std::cout << "All players connected! Starting match." << std::endl;
  }
}