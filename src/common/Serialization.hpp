#pragma once
#include <vector>
#include <cstring>
#include "common/Packets.hpp"
/**    * @file Serialization.hpp
    * @brief Utility functions for serializing and deserializing packet structures.
    */  
// Serialize any POD struct -> vector<uint8_t>
template <typename T>
std::vector<uint8_t> serialize(const T& obj) {
    std::vector<uint8_t> buffer(sizeof(T));
    std::memcpy(buffer.data(), &obj, sizeof(T));
    return buffer;
}
/** Deserialize vector<uint8_t> -> any POD struct
  * Note: Caller must ensure buffer size is at least sizeof(T)
  */    
template <typename T>
T deserialize(const std::vector<uint8_t>& buffer) {
    T obj{};
    std::memcpy(&obj, buffer.data(), sizeof(T));
    return obj;
}

// INPUT PACKET (InputPacket + UserCmd[])
inline std::vector<uint8_t> serializeInput(const InputPacket& pkt, const std::vector<UserCmd>& cmds) {
    std::vector<uint8_t> buffer(sizeof(InputPacket) + cmds.size() * sizeof(UserCmd));
    std::memcpy(buffer.data(), &pkt, sizeof(InputPacket));
    if (!cmds.empty()) {
        std::memcpy(buffer.data() + sizeof(InputPacket), cmds.data(), cmds.size() * sizeof(UserCmd));
    }
    return buffer;
}

inline void deserializeInput(const std::vector<uint8_t>& buffer, InputPacket& pkt, std::vector<UserCmd>& cmds) {
    std::memcpy(&pkt, buffer.data(), sizeof(InputPacket));
    cmds.resize(pkt.cmdCount);
    if (pkt.cmdCount > 0) {
        std::memcpy(cmds.data(), buffer.data() + sizeof(InputPacket), pkt.cmdCount * sizeof(UserCmd));
    }
}

// SNAPSHOT (SnapshotHeader + EntityState[])
inline std::vector<uint8_t> serializeSnapshot(const SnapshotHeader& header, const std::vector<EntityState>& entities) {
    std::vector<uint8_t> buffer(sizeof(SnapshotHeader) + entities.size() * sizeof(EntityState));
    std::memcpy(buffer.data(), &header, sizeof(SnapshotHeader));
    if (!entities.empty()) {
        std::memcpy(buffer.data() + sizeof(SnapshotHeader), entities.data(), entities.size() * sizeof(EntityState));
    }
    return buffer;
}

inline void deserializeSnapshot(const std::vector<uint8_t>& buffer, SnapshotHeader& header, std::vector<EntityState>& entities) {
    std::memcpy(&header, buffer.data(), sizeof(SnapshotHeader));
    entities.resize(header.entityCount);
    if (header.entityCount > 0) {
        std::memcpy(entities.data(), buffer.data() + sizeof(SnapshotHeader), header.entityCount * sizeof(EntityState));
    }
}