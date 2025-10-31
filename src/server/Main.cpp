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
 #include <iostream>
 #include <string>
 #include <cstring>
 #include <stdexcept> 
 
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
        server s(port);
        s.run();
     }
     catch (const std::exception &e)
     {
         std::cerr << "Server error: " << e.what() << std::endl;
         return 1;
     }
 
     return 0;
 }