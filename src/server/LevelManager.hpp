#pragma once
#include "engine/Engine.hpp"
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <unordered_set>
#include <nlohmann/json.hpp>

class server;    
struct PlayerInfo;
class LevelManager {
public:
    LevelManager(engine::registry &registry, engine::net::UdpSocket &socket,
                 std::vector<PlayerInfo> &players, uint32_t &tick,
                 std::unordered_set<uint32_t> &liveEntities);

    void update();
    void startNextLevel();

private:
    void loadLevelFile(size_t index, nlohmann::json &out);
    void spawnEntities(const nlohmann::json &levelJson);
    void notifyLevelStart(uint32_t level);
    void notifyLevelEnd(uint32_t level);

    engine::registry &_registry;
    engine::net::UdpSocket &_socket;
    std::vector<PlayerInfo> &_players;
    uint32_t &_tick;
    std::unordered_set<uint32_t> &_liveEntities;

    uint32_t _levelStartTick = 0;
    uint32_t _currentLevel = 1;
    uint32_t _levelDuration = 10 * 60;
};
