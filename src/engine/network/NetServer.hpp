#pragma once

#include <string>
#include <memory>
#include <functional>
#include <thread>

#include "IoContext.hpp"
#include "UdpSocket.hpp"

namespace engine::net
{
    class NetServer
    {
        public:
            using PacketHandler = std::function<void(const Endpoint &,
                const PacketHeader &, const std::vector<uint8_t> &)>;

            NetServer(unsigned short port = 4242, std::string ip = "127.0.0.1");
            ~NetServer();

            void start();
            void stop();
            void poll();

            void set_packet_handler(PacketHandler handler);

            void send(const PacketHeader &hdr,
                const std::vector<uint8_t> &payload,
                const Endpoint &endpoint);

            void broadcast(const PacketHeader &hdr,
                const std::vector<uint8_t> &payload,
                const std::vector<Endpoint> &endpoints);

            bool running() const { return _running; }
            std::string detect_local_ip();
        private:
            std::string _ip;
            unsigned short _port;
            bool _running = false;
            engine::net::IoContext _io;
            engine::net::UdpSocket _socket;
            PacketHandler _handler;
            std::thread _ioThread;
    };
}
