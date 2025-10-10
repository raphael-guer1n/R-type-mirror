#pragma once
#include <memory>
#include <unordered_set>
#include <asio.hpp>
#include "Hud.hpp"
#include "Menu.hpp"
#include "Player.hpp"
#include "engine/network/Udpsocket.hpp"
#include "engine/renderer/App.hpp"
#include "engine/events/Events.hpp"
#include "engine/ecs/Registry.hpp"
#include "Background.hpp"

namespace R_Type
{
    class Hud;
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
            uint8_t keyToBit(engine::R_Events::Key key);
        private:
            std::unique_ptr<asio::ip::udp::endpoint> _serverEndpoint;
            uint32_t _tick = 0;
            uint8_t _keys = 0;
            uint32_t _player = 0;
            std::unordered_set<uint32_t> _activeEntities;
            asio::ip::udp::endpoint _sender;
            engine::R_Graphic::App _app;
            engine::registry _registry;
            std::unique_ptr<Background> _background;
            asio::io_context _ioContext;
            std::unique_ptr<engine::net::UdpSocket> _client;
            std::unique_ptr<Player> _playerTexture;
            std::unique_ptr<Hud> _hud;
            std::unique_ptr<R_Type::Menu> _menu;
            bool _inMenu = true;
    };
}
