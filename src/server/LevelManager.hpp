/**
 * @file LevelManager.hpp
 * @brief Declares the LevelManager class responsible for managing level progression, loading, and events.
 *
 * The LevelManager handles level sequencing, entity spawning, and communication between the server
 * and connected clients during gameplay. It loads level data from JSON files, spawns entities into
 * the ECS registry, and notifies players of level transitions.
 *
 * @date 2025
 * @author
 *  Lisa Goulmot
 */
#pragma once
#include "engine/Engine.hpp"
#include "engine/network/NetServer.hpp"
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <unordered_set>
#include <nlohmann/json.hpp>

/**
 * @class LevelManager
 * @brief Manages level loading, progression, and synchronization across the network.
 *
 * The LevelManager coordinates level state between the ECS registry and the network layer.
 * It tracks the current level, controls when levels start and end, and spawns entities defined
 * in external JSON level files.
 *
 * ### Responsibilities:
 * - Load level data from JSON files.
 * - Spawn entities (enemies, obstacles, etc.) defined per level.
 * - Notify clients when a level starts or ends.
 * - Handle level timing and transitions.
 *
 * ### Usage:
 * The server creates a LevelManager instance and calls `update()` each tick to manage
 * level state and progression.
 */
class server;
struct PlayerInfo;
class LevelManager {
public:
    LevelManager(engine::registry &registry, engine::net::NetServer &socket,
                 std::vector<PlayerInfo> &players, uint32_t &tick,
                 std::unordered_set<uint32_t> &liveEntities);

    void update();
    void startNextLevel();
    bool _noMoreLevels = false;
private:
    void loadLevelFile(size_t index, nlohmann::json &out);
    void spawnEntities(const nlohmann::json &levelJson);
    void notifyLevelStart(uint32_t level);
    void notifyLevelEnd(uint32_t level);

    engine::registry &_registry;
    engine::net::NetServer &_server;
    std::vector<PlayerInfo> &_players;
    uint32_t &_tick;
    std::unordered_set<uint32_t> &_liveEntities;

    uint32_t _levelStartTick = 0;
    uint32_t _currentLevel = 1;
    uint32_t _levelDuration = 10 * 60;
};