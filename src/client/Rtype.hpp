#pragma once
#include <memory>
#include <unordered_set>
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
            void receiveSnapshot();
            void draw();
            engine::R_Graphic::App& getApp();
            engine::registry& getRegistry();
        public:
            void waiting_connection();
        private:
            std::unique_ptr<asio::ip::udp::endpoint> _serverEndpoint;
            uint32_t tick = 0;
            uint8_t keys = 0;
            uint32_t _player = 0;
            std::unordered_set<uint32_t> _activeEntities;
            asio::ip::udp::endpoint _sender;
            engine::R_Graphic::App _app;
            engine::registry _registry;
            std::unique_ptr<Background> _background;
            std::unique_ptr<engine::net::UdpSocket> _client;
    };
}
