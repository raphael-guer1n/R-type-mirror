#include <asio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <chrono>

#include "engine/network/Udpsocket.hpp"
#include "engine/ecs/Registry.hpp"
#include "common/Components.hpp"
#include "common/Components_client_sfml.hpp"
#include "common/Packets.hpp"
#include "engine/ecs/iterator/Indexed_zipper.hpp"
#include "common/Systems_client_sfml.hpp"
#include "common/Systems.hpp"
#include "engine/renderer/Error.hpp"
#include "Rtype.hpp"

int main() {
    try
    {
        R_Type::Rtype game;
        game.waiting_connection();
        game.getApp().run(
            [&game](float dt, const std::vector<R_Events::Event> &events)
            {
                game.update(dt, events);
            },
            [&game]() { game.draw(); }
        );
    } catch(const R_Graphic::Error& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}