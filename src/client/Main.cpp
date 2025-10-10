#include "engine/renderer/Error.hpp"
#include "engine/events/Events.hpp"
#include "Rtype.hpp"

int main(int argc, char* argv[]) {
    try
    {
        R_Type::Rtype game;
        game.getApp().run(
            [&game](float dt, const std::vector<engine::R_Events::Event> &events)
            {
                game.update(dt, events);
            },
            [&game]() { game.draw(); }
        );
    } catch(const engine::R_Graphic::Error& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}