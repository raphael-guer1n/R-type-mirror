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

/**
 * @class LevelManager
 * @brief Manages level data, transitions, and synchronization in the R-Type server.
 *
 * The LevelManager is responsible for loading level definitions, spawning entities,
 * and notifying clients about level changes. It interacts closely with the ECS registry,
 * the networking subsystem, and player data structures.
 *
 * @note The LevelManager operates primarily on the server side to ensure all connected
 * clients remain synchronized with the current level state.
 */
class LevelManager {
public:
    /**
     * @brief Constructs a new LevelManager instance.
     *
     * Initializes the manager with references to the ECS registry, network socket,
     * player list, server tick counter, and the list of active entities.
     *
     * @param registry Reference to the ECS registry used for entity management.
     * @param socket Reference to the UDP socket used for server-client communication.
     * @param players Reference to the list of connected player information structures.
     * @param tick Reference to the current server tick counter.
     * @param liveEntities Reference to the set of currently active entity IDs.
     */
    LevelManager(engine::registry &registry,
                 engine::net::UdpSocket &socket,
                 std::vector<PlayerInfo> &players,
                 uint32_t &tick,
                 std::unordered_set<uint32_t> &liveEntities);

    /**
     * @brief Updates the level state.
     *
     * Called periodically (e.g., once per game tick) to handle level progression,
     * check for transitions, and trigger level end/start events.
     */
    void update();

    /**
     * @brief Starts the next level.
     *
     * Increments the level index, loads the next level file, and respawns entities.
     * If no additional levels exist, sets @_noMoreLevels to true.
     */
    void startNextLevel();

    /** @brief Indicates whether there are no more levels to load. */
    bool _noMoreLevels = false;

private:
    /**
     * @brief Loads a level definition file and parses its JSON content.
     *
     * @param index Index of the level to load.
     * @param out Reference to a JSON object where the parsed data will be stored.
     *
     * @throws std::runtime_error if the file cannot be opened or parsed.
     */
    void loadLevelFile(size_t index, nlohmann::json &out);

    /**
     * @brief Spawns entities defined in a level JSON file.
     *
     * Creates ECS entities and components as defined in the provided JSON configuration.
     *
     * @param levelJson The parsed JSON content of the level file.
     */
    void spawnEntities(const nlohmann::json &levelJson);

    /**
     * @brief Sends a network notification to clients indicating that a new level has started.
     *
     * @param level ID or index of the level that has just started.
     */
    void notifyLevelStart(uint32_t level);

    /**
     * @brief Sends a network notification to clients indicating that a level has ended.
     *
     * @param level ID or index of the level that has just finished.
     */
    void notifyLevelEnd(uint32_t level);

    /** @brief Reference to the ECS registry used to manage entities and components. */
    engine::registry &_registry;
    engine::net::NetServer &_server;

    /** @brief Reference to the UDP socket used for server-client communication. */
    engine::net::UdpSocket &_socket;

    /** @brief Reference to the list of connected players. */
    std::vector<PlayerInfo> &_players;

    /** @brief Reference to the global tick counter used for timing and synchronization. */
    uint32_t &_tick;

    /** @brief Reference to the set of currently active entity IDs. */
    std::unordered_set<uint32_t> &_liveEntities;

    /** @brief Tick count when the current level started. */
    uint32_t _levelStartTick = 0;

    /** @brief The index or ID of the currently active level. */
    uint32_t _currentLevel = 1;

    /** @brief Duration of the current level in ticks (default: 10 × 60). */
    uint32_t _levelDuration = 10 * 60;
};
