#pragma once
#include <memory>
// #include "Player.hpp"
#include "engine/network/Udpsocket.hpp"
#include "engine/renderer/App.hpp"
#include "engine/ecs/Registry.hpp"
#include "Background.hpp"

namespace R_Type
{
    class Rtype
    {
        public:
            Rtype();
            ~Rtype() = default;
            void update(float deltaTime, const std::vector<engine::R_Events::Event> &events);
            void draw();
            engine::R_Graphic::App& getApp();
            engine::registry& getRegistry();
        public:
            void waiting_connection();
        private:
            engine::R_Graphic::App _app;
            engine::registry _registry;
            std::unique_ptr<Background> _background;
            std::unique_ptr<engine::net::UdpSocket> _client;
            uint32_t _myEntity;
    };
}
