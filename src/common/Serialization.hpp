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
