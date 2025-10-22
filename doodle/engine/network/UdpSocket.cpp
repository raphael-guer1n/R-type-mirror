#include "engine/network/UdpSocket.hpp"

#include <asio.hpp>
#include "engine/network/detail/IoContextInternal.hpp"
#include <iostream>
#include <cstring>

namespace engine::net
{

    static asio::ip::udp::endpoint to_asio_endpoint(const Endpoint &ep)
    {
        return asio::ip::udp::endpoint(asio::ip::make_address(ep.address), ep.port);
    }

    static Endpoint from_asio_endpoint(const asio::ip::udp::endpoint &ep)
    {
        return Endpoint{ep.address().to_string(), ep.port()};
    }

    class UdpSocketImpl
    {
    public:
        explicit UdpSocketImpl(IoContext &io, unsigned short port)
            : socket(*static_cast<asio::io_context *>(io.native_handle()),
                     asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
        {
            socket.non_blocking(true);
        }

        asio::ip::udp::socket socket;
    };

    UdpSocket::UdpSocket(IoContext &ctx, unsigned short localPort)
        : _impl(std::make_unique<UdpSocketImpl>(ctx, localPort))
    {
        std::cout << "UDP socket bound on port " << localPort << "\n";
    }

    UdpSocket::~UdpSocket() = default;
    UdpSocket::UdpSocket(UdpSocket &&) noexcept = default;
    UdpSocket &UdpSocket::operator=(UdpSocket &&) noexcept = default;

    void UdpSocket::sendRaw(const void *data, std::size_t size, const Endpoint &endpoint)
    {
        _impl->socket.send_to(asio::buffer(data, size), to_asio_endpoint(endpoint));
    }

    void UdpSocket::send(const PacketHeader &header, const std::vector<std::uint8_t> &payload,
                         const Endpoint &endpoint)
    {
        std::vector<std::uint8_t> buffer(sizeof(PacketHeader) + payload.size());
        std::memcpy(buffer.data(), &header, sizeof(PacketHeader));
        if (!payload.empty())
            std::memcpy(buffer.data() + sizeof(PacketHeader), payload.data(), payload.size());
        sendRaw(buffer.data(), buffer.size(), endpoint);
    }

    std::optional<std::pair<PacketHeader, std::vector<std::uint8_t>>>
    UdpSocket::receive(Endpoint &sender)
    {
        std::array<std::uint8_t, 1500> buf{};
        asio::ip::udp::endpoint from;
        asio::error_code ec;
        std::size_t bytes = _impl->socket.receive_from(asio::buffer(buf), from, 0, ec);

        if (ec)
        {
            if (ec == asio::error::would_block || ec == asio::error::try_again)
                return std::nullopt;
            std::cerr << "UDP receive error: " << ec.message() << "\n";
            return std::nullopt;
        }
        if (bytes < sizeof(PacketHeader))
            return std::nullopt;

        sender = from_asio_endpoint(from);
        PacketHeader hdr{};
        std::memcpy(&hdr, buf.data(), sizeof(PacketHeader));
        std::vector<std::uint8_t> payload(bytes - sizeof(PacketHeader));
        if (!payload.empty())
            std::memcpy(payload.data(), buf.data() + sizeof(PacketHeader), payload.size());
        return std::make_optional(std::make_pair(hdr, std::move(payload)));
    }

} // namespace engine::net
