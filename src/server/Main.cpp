/**
 * @file Main.cpp
 * @brief Entry point for the R-Type server.
 *
 * Usage:
 *   ./r-type_server [PORT]
 *
 * Example:
 *   ./r-type_server 4242
 *
 * The server binds to the given port and prints the host's IP address.
 */

#include "server/Server.hpp"
#include "engine/network/IoContext.hpp"
#include "engine/profiling/Profiler.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main(int argc, char* argv[])
{
    unsigned short port = 4242;
    if (argc >= 2)
    {
        try {
            port = static_cast<unsigned short>(std::stoi(argv[1]));
        } catch (...) {
            std::cerr << "Invalid port argument. Using default 4242.\n";
            port = 4242;
        }
    }
    try
    {
        engine::net::IoContext io;
        server s(io, port);
        char hostname[256];
        std::string ip = "127.0.0.1";
        if (gethostname(hostname, sizeof(hostname)) == 0)
        {
            addrinfo hints{};
            addrinfo* info = nullptr;
            hints.ai_family = AF_INET;

            if (getaddrinfo(hostname, nullptr, &hints, &info) == 0 && info)
            {
                sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(info->ai_addr);
                ip = inet_ntoa(addr->sin_addr);
                freeaddrinfo(info);
            }
        }

        std::cout << "IP Address: " << ip << "\n";
        std::cout << "Port: " << port << "\n";
        std::cout << "[Profiling] Server profiling enabled. Stats will be logged periodically.\n";
        
        s.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
