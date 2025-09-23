/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** serializer
*/

#pragma once
#include <vector>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <stdexcept>

namespace protocol {

    template<typename T>
    std::vector<uint8_t> serialize(const T& data) {
        static_assert(std::is_trivially_copyable_v<T>, 
                    "serialize<T> requires trivially copyable type");

        std::vector<uint8_t> buffer(sizeof(T));
        std::memcpy(buffer.data(), &data, sizeof(T));
        return buffer;
    }

    template<typename T>
    T deserialize(const std::vector<uint8_t>& buffer, std::size_t offset = 0) {
        static_assert(std::is_trivially_copyable_v<T>, 
                    "deserialize<T> requires trivially copyable type");

        if (buffer.size() < offset + sizeof(T)) {
            throw std::runtime_error("deserialize<T>: buffer too small");
        }

        T obj;
        std::memcpy(&obj, buffer.data() + offset, sizeof(T));
        return obj;
    }

}
