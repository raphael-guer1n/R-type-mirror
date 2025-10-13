#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "engine/network/Endpoint.hpp"
#include "engine/network/IoContext.hpp"
#include "common/Packets.hpp"

namespace engine::net
{

    class UdpSocketImpl; // hidden implementation using Asio

    class UdpSocket
    {
    public:
        // Binds a UDP socket on the given local port (0 for ephemeral)
        UdpSocket(IoContext &ctx, unsigned short localPort);
        ~UdpSocket();
        UdpSocket(UdpSocket &&) noexcept;
        UdpSocket &operator=(UdpSocket &&) noexcept;
        UdpSocket(const UdpSocket &) = delete;
        UdpSocket &operator=(const UdpSocket &) = delete;

        // Send raw bytes to a remote endpoint
        void sendRaw(const void *data, std::size_t size, const Endpoint &endpoint);

        // Send header + payload convenience
        void send(const PacketHeader &header, const std::vector<std::uint8_t> &payload,
                  const Endpoint &endpoint);

        // Non-blocking receive; returns header+payload when data is available.
        // Fills 'sender' with the packet source.
        std::optional<std::pair<PacketHeader, std::vector<std::uint8_t>>> receive(Endpoint &sender);

    private:
        std::unique_ptr<UdpSocketImpl> _impl;
    };

} // namespace engine::net
