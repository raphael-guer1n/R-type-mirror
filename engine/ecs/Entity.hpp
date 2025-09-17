#pragma once
#include <cstddef>

namespace engine
{
    class entity_t
    {
    public:
        explicit entity_t(std::size_t id) : _id(id) {}

        entity_t(const entity_t &) = default;
        entity_t(entity_t &&) noexcept = default;
        entity_t &operator=(const entity_t &) = default;
        entity_t &operator=(entity_t &&) noexcept = default;

        operator std::size_t() const noexcept { return _id; }

        bool operator==(const entity_t &other) const noexcept
        {
            return _id == other._id;
        }
        bool operator!=(const entity_t &other) const noexcept
        {
            return _id != other._id;
        }

    private:
        std::size_t _id;
    };
}
