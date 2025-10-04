#pragma once

#include <asio.hpp>
#include <vector>
#include <optional>
#include <cstring>
#include <iostream>

#include "common/Packets.hpp"
/**
 * @brief UDP socket wrapper for sending and receiving packets using ASIO.
 *
 * This class provides a simple interface for sending and receiving raw data or
 * structured packets (with headers and payloads) over UDP. It is designed to be
 * non-blocking and integrates with ASIO's io_context for asynchronous network operations.
 *
 * Usage:
 *  - Construct with an io_context and port to bind the socket.
 *  - Use sendRaw() to send arbitrary data.
 *  - Use send() to send a PacketHeader and payload.
 *  - Use receive() to receive packets; returns std::nullopt if no data is available.
 *
 * Note: Error handling is minimal; production code should handle exceptions and errors robustly.
 */
namespace engine
{
    namespace net
    {

        class UdpSocket
        {
        public:
            UdpSocket(asio::io_context &ctx, unsigned short port)
                : socket_(ctx, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
            {
                socket_.non_blocking(true);
                std::cout << "UDP socket bound on port " << port << "\n";
            }

            // Send a raw buffer
            void sendRaw(const void *data, size_t size,
                         const asio::ip::udp::endpoint &endpoint)
            {
                socket_.send_to(asio::buffer(data, size), endpoint);
            }

            // Send a packet with header+payload
            void send(const PacketHeader &header, const std::vector<uint8_t> &data,
                      const asio::ip::udp::endpoint &endpoint)
            {
                std::vector<uint8_t> buffer(sizeof(PacketHeader) + data.size());
                std::memcpy(buffer.data(), &header, sizeof(PacketHeader));
                if (!data.empty())
                    std::memcpy(buffer.data() + sizeof(PacketHeader), data.data(),
                                data.size());
                sendRaw(buffer.data(), buffer.size(), endpoint);
            }

            // Blocking receive (we’ll wrap into non-blocking in server logic)
            std::optional<std::pair<PacketHeader, std::vector<uint8_t>>> receive(
                asio::ip::udp::endpoint &sender)
            {
                std::array<uint8_t, 1500> buf{};
                asio::error_code ec;
            
                size_t bytes = socket_.receive_from(asio::buffer(buf), sender, 0, ec);
            
                if (ec) {
                    if (ec == asio::error::would_block || ec == asio::error::try_again) {
                        return std::nullopt; // no packet this tick
                    } else {
                        std::cerr << "UDP receive error: " << ec.message() << "\n";
                        return std::nullopt;
                    }
                }
            
                if (bytes < sizeof(PacketHeader)) {
                    std::cerr << "UDP packet too small\n";
                    return std::nullopt;
                }
            
                PacketHeader hdr;
                std::memcpy(&hdr, buf.data(), sizeof(PacketHeader));
            
                std::vector<uint8_t> payload;
                payload.resize(bytes - sizeof(PacketHeader));
                if (!payload.empty())
                    std::memcpy(payload.data(), buf.data() + sizeof(PacketHeader), payload.size());
            
                return std::make_pair(hdr, std::move(payload));
            }

        private:
            asio::ip::udp::socket socket_;
        };

    } // namespace net
} // namespace engine