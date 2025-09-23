/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** network
*/

#pragma once
#include <asio.hpp>
#include <string>
#include <vector>
#include "protocol/Packet.hpp"
#include "protocol/Serializer.hpp"

using protocol::PacketType;
using protocol::PacketHeader;
using protocol::serialize;
using protocol::deserialize;

namespace engine {
    class Network {
    public:
        Network(const std::string& host, unsigned short port);
        Network() : _socket(_io) {}

        void bind(unsigned short port);

        void sendRaw(const std::vector<uint8_t>& data,
                     const asio::ip::udp::endpoint& target);

        void receiveRaw(std::vector<uint8_t>& buffer,
                        asio::ip::udp::endpoint& sender);

        template<typename T>
        void sendPacket(PacketType type, const T& payload,
                        const asio::ip::udp::endpoint& target) {
            auto body = serialize(payload);
            auto header = makeHeader(type, body.size(), _seq++);
            auto head = serialize(header);
            std::vector<uint8_t> packet;
            packet.reserve(head.size() + body.size());
            packet.insert(packet.end(), head.begin(), head.end());
            packet.insert(packet.end(), body.begin(), body.end());
            sendRaw(packet, target);
        }

        template<typename T>
        T receivePacket(PacketType expectedType, asio::ip::udp::endpoint& sender) {
            std::vector<uint8_t> buf(2048);
            receiveRaw(buf, sender);
            auto header = deserialize<PacketHeader>(
                std::vector<uint8_t>(buf.begin(), buf.begin() + sizeof(PacketHeader))
            );
            if (header.type != static_cast<uint16_t>(expectedType))
                throw std::runtime_error("Unexpected packet type");
            auto payload = deserialize<T>(
                std::vector<uint8_t>(buf.begin() + sizeof(PacketHeader),
                                     buf.begin() + sizeof(PacketHeader) + sizeof(T))
            );
            return payload;
        }

    private:
        PacketHeader makeHeader(PacketType type, uint16_t payloadSize, uint32_t seq) const;

        asio::io_context _io;
        asio::ip::udp::socket _socket;
        asio::ip::udp::endpoint _endpoint;
        uint32_t _seq = 0;
    };
}
