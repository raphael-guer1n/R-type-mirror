#include <iostream>
#include "NetServer.hpp"

#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif

engine::net::NetServer::NetServer(unsigned short port, std::string ip)
: _port(port), _ip(std::move(ip)), _socket(_io, _port)
{
}

engine::net::NetServer::~NetServer()
{
    stop();
}

void engine::net::NetServer::start()
{
    if (_running)
        return;
    _running = true;
    std::cout << "Server started on " << detect_local_ip() << ":" << _port << "\n";
    _ioThread = std::thread([this]() {
        _io.run();
    });
}

void engine::net::NetServer::stop()
{
    if (!_running)
        return;
    _running = false;
    _io.stop();
    if (_ioThread.joinable())
        _ioThread.join();
    std::cout << "Server stopped\n";
}

void engine::net::NetServer::poll()
{
    if (!_running)
        return;
    PacketHeader hdr;
    std::vector<uint8_t> payload;
    Endpoint sender;
    while (_socket.PollPacket(hdr, payload, sender)) {
        if (_handler)
            _handler(sender, hdr, payload);
    }
}

void engine::net::NetServer::set_packet_handler(PacketHandler handler)
{
    _handler = std::move(handler);
}

void engine::net::NetServer::send(const PacketHeader &hdr,
    const std::vector<uint8_t> &payload, const Endpoint &endpoint)
{
    _socket.send(hdr, payload, endpoint);
}

void engine::net::NetServer::broadcast(const PacketHeader &hdr,
    const std::vector<uint8_t> &payload,
    const std::vector<Endpoint> &endpoints)
{
    for (const auto &ep : endpoints)
        send(hdr, payload, ep);
}

std::string engine::net::NetServer::detect_local_ip()
{
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0)
        return "127.0.0.1";
    addrinfo hints{};
    hints.ai_family = AF_INET;
    addrinfo *info = nullptr;
    if (getaddrinfo(hostname, nullptr, &hints, &info) != 0 || !info)
        return "127.0.0.1";
    sockaddr_in *addr = reinterpret_cast<sockaddr_in *>(info->ai_addr);
    std::string ip = inet_ntoa(addr->sin_addr);
    freeaddrinfo(info);
    return ip;
}

