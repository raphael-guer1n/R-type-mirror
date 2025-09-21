/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** Serializer
*/

#pragma once
#include <vector>
#include <cstring>

template<typename T>
std::vector<uint8_t> serialize(const T& data) {
    std::vector<uint8_t> buffer(sizeof(T));
    std::memcpy(buffer.data(), &data, sizeof(T));
    return buffer;
}

template<typename T>
T deserialize(const std::vector<uint8_t>& buffer) {
    T data;
    std::memcpy(&data, buffer.data(), sizeof(T));
    return data;
}
