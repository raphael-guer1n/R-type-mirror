#include "engine/network/NetClient.hpp"
#include <iostream>

using namespace engine::net;

NetClient::NetClient(const std::string &serverIp, unsigned short serverPort)
    : _serverIp(serverIp),
      _serverPort(serverPort),
      _socket(_io, 0),
      _serverEndpoint(make_endpoint(serverIp, serverPort))
{
}

NetClient::~NetClient()
{
    stop();
}

void NetClient::start()
{
    if (_running)
        return;
    _running = true;

    _ioThread = std::thread([this]() { _io.run(); });

    std::cout << "Client started, connecting to " << _serverIp << ":" << _serverPort << "\n";
}

void NetClient::stop()
{
    if (!_running)
        return;
    _running = false;

    _io.stop();
    if (_ioThread.joinable())
        _ioThread.join();

    std::cout << "Client stopped\n";
}

void NetClient::poll()
{
    if (!_running)
        return;

    PacketHeader hdr;
    std::vector<uint8_t> payload;
    Endpoint sender;

    while (_socket.PollPacket(hdr, payload, sender)) {
        if (hdr.seq != 0) {
            PacketHeader ackHdr{ACK, sizeof(uint32_t), 0};
            std::vector<uint8_t> ackPayload(sizeof(uint32_t));
            std::memcpy(ackPayload.data(), &hdr.seq, sizeof(uint32_t));
            _socket.send(ackHdr, ackPayload, sender);
        }
        if (_handler)
            _handler(hdr, payload);
    }
}

void NetClient::set_packet_handler(PacketHandler handler)
{
    _handler = std::move(handler);
}

void NetClient::send(const PacketHeader &hdr, const std::vector<uint8_t> &payload)
{
    _socket.send(hdr, payload, _serverEndpoint);
}
