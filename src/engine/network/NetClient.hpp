#pragma once
#include "engine/network/IoContext.hpp"
#include "engine/network/UdpSocket.hpp"
#include "common/Packets.hpp"
#include <functional>
#include <thread>
#include <atomic>

namespace engine::net
{
    class NetClient
    {
    public:
        using PacketHandler = std::function<void(const PacketHeader &, const std::vector<uint8_t> &)>;

        NetClient(const std::string &serverIp, unsigned short serverPort);
        ~NetClient();

        void start();
        void stop();
        void poll();

        void set_packet_handler(PacketHandler handler);

        void send(const PacketHeader &hdr, const std::vector<uint8_t> &payload);

        bool running() const { return _running; }

    private:
        std::string _serverIp;
        unsigned short _serverPort;
        bool _running = false;

        engine::net::IoContext _io;
        engine::net::UdpSocket _socket;
        engine::net::Endpoint _serverEndpoint;
        PacketHandler _handler;
        std::thread _ioThread;
    };
}
