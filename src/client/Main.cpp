#include "Rtype.hpp"
#include "engine/renderer/Error.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    std::string serverIp = "127.0.0.1";
    unsigned short port = 4242;

    if (argc >= 2)
        serverIp = argv[1];
    if (argc >= 3)
    {
        try {
            port = static_cast<unsigned short>(std::stoi(argv[2]));
        } catch (...) {
            std::cerr << "Invalid port argument. Using default 4242\n";
        }
    }
    try
    {
        R_Type::Rtype game;

        game.setServerEndpoint(serverIp, port);
        game.getApp().run(
            [&game](float dt, const std::vector<engine::R_Events::Event> &events)
            {
                game.update(dt, events);
            },
            [&game]() { game.draw(); }
        );
    }
    catch (const engine::R_Graphic::Error& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
