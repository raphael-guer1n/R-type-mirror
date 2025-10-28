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

        std::cout << "Server Address: localhost (127.0.0.1)\n";
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
