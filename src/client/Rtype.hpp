#pragma once
#include <memory>
#include <unordered_set>
#include <asio.hpp>
#include "Player.hpp"
#include "engine/network/Udpsocket.hpp"
#include "engine/renderer/App.hpp"
#include "engine/events/Events.hpp"
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
            uint32_t _tick = 0;
            std::unordered_set<engine::R_Events::Key> _pressedKeys;
            uint32_t _player = 0;
            std::unordered_set<uint32_t> _activeEntities;
            asio::ip::udp::endpoint _sender;
            engine::R_Graphic::App _app;
            engine::registry _registry;
            std::unique_ptr<Background> _background;
            asio::io_context _ioContext;
            std::unique_ptr<engine::net::UdpSocket> _client;
            std::unique_ptr<Player> _playerData;
            std::unordered_map<uint32_t, size_t> _entityMap;
    };
    void setAnimation(component::animation &anim, const std::string &clip, bool reverse);
}
