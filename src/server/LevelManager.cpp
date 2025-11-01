#include "LevelManager.hpp"
#include "Server.hpp"
#include "EnemyConfig.hpp"
#include "common/Packets.hpp"
#include "ServerUtils.hpp"
#include <fstream>
#include <iostream>
using json = nlohmann::json;

LevelManager::LevelManager(engine::registry &registry, engine::net::UdpSocket &socket,
                           std::vector<PlayerInfo> &players, uint32_t &tick,
                           std::unordered_set<uint32_t> &liveEntities)
    : _registry(registry), _socket(socket), _players(players), _tick(tick),
      _liveEntities(liveEntities)
{
    startNextLevel();
}

void LevelManager::loadLevelFile(size_t index, json &out)
{
    std::string path = "configs/levels/level" + std::to_string(index) + ".json";
    std::ifstream f(path);

    if (!f.is_open())
        throw std::runtime_error("Cannot open level file: " + path);
    f >> out;
}

void LevelManager::startNextLevel()
{
    json level;
    try {
        loadLevelFile(_currentLevel, level);
    } catch (...) {
        std::cout << "No more levels!\n";
        _noMoreLevels = true; 
        return;
    }
    _levelDuration = level["duration"].get<uint32_t>() * 60;
    _levelStartTick = _tick;
    notifyLevelStart(_currentLevel);
    spawnEntities(level);
}

void LevelManager::spawnEntities(const json &levelJson)
{
    for (auto &entry : levelJson["entities"])
    {
        std::string cfgPath = entry["config"]; 
        float x = entry["x"];
        float y = entry["y"];
        float velX = entry["velocityX"];
        float velY = entry["velocityY"];

        try {
            EnemyConfig cfg = EnemyConfig::load_enemy_config(cfgPath);

            auto e = _registry.spawn_entity();
            _liveEntities.insert(static_cast<uint32_t>(e));

            _registry.add_component(e, component::position{x, y});
            _registry.add_component(e, component::velocity{velX, velY});
            _registry.add_component<component::hitbox>(e, std::move(cfg.hitbox));
            _registry.add_component(e, component::entity_kind::enemy);
            _registry.add_component(e, component::collision_state{false});
            _registry.add_component(e, component::health{(uint8_t)cfg.hp});

            component::ai_controller ai;
            ai.behavior = cfg.behavior;
            ai.speed = cfg.speed;
            _registry.add_component<component::ai_controller>(e, std::move(ai));

            if (!cfg.spells.empty())
            {
                component::spellbook sb;
                sb.spells = cfg.spells;
                _registry.add_component<component::spellbook>(e, std::move(sb));
            }

            std::cout << "[LEVEL " << _currentLevel << "] Spawned " << cfgPath << " at x=" << x << ", y=" << y << "\n";
        }
        catch (const std::exception &ex)
        {
            std::cerr << "Failed to spawn entity from config: " << ex.what() << "\n";
        }
    }
}

void LevelManager::update()
{
    if (_tick - _levelStartTick >= _levelDuration)
    {
        notifyLevelEnd(_currentLevel);
        _currentLevel++;
        startNextLevel();
    }
}

void LevelManager::notifyLevelStart(uint32_t level)
{
    PacketHeader hdr{LEVEL_START, sizeof(LevelStartPayload), 0};
    LevelStartPayload p{level};
    for (auto &pl : _players)
        _socket.send(hdr, std::vector<uint8_t>((uint8_t *)&p, (uint8_t *)&p + sizeof(p)), pl.endpoint);
}

void LevelManager::notifyLevelEnd(uint32_t level)
{
    PacketHeader hdr{LEVEL_END, sizeof(LevelEndPayload), 0};
    LevelEndPayload p{level};
    for (auto &pl : _players)
        _socket.send(hdr, std::vector<uint8_t>((uint8_t *)&p, (uint8_t *)&p + sizeof(p)), pl.endpoint);
}
