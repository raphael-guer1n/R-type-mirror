/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** network
*/

#include "network/Network.hpp"

using asio::ip::udp;

namespace engine {
    Network::Network(const std::string& host, unsigned short port)
        : _socket(_io, udp::endpoint(udp::v4(), 0)) {
        _endpoint = udp::endpoint(asio::ip::make_address(host), port);
    }

    void Network::bind(unsigned short port) {
        _socket.open(udp::v4());
        _socket.bind(udp::endpoint(udp::v4(), port));
    }

    void Network::sendRaw(const std::vector<uint8_t>& data,
                          const udp::endpoint& target) {
        _socket.send_to(asio::buffer(data), target);
    }

    void Network::receiveRaw(std::vector<uint8_t>& buffer,
                             udp::endpoint& sender) {
        sender = udp::endpoint();
        size_t len = _socket.receive_from(asio::buffer(buffer), sender);
        buffer.resize(len);
    }

    PacketHeader Network::makeHeader(PacketType type, uint16_t payloadSize, uint32_t seq) const {
        PacketHeader h{};
        h.type = static_cast<uint16_t>(type);
        h.size = payloadSize;
        h.seq = seq;
        return h;
    }
}
