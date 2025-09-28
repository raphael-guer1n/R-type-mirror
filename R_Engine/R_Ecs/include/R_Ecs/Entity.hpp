#pragma once
#include <cstddef>

namespace R_Ecs
{
    class Entity
    {
    public:
        explicit Entity(std::size_t id) : _id(id) {}

        Entity(const Entity &) = default;
        Entity(Entity &&) noexcept = default;
        Entity &operator=(const Entity &) = default;
        Entity &operator=(Entity &&) noexcept = default;

        operator std::size_t() const noexcept { return _id; }

        bool operator==(const Entity &other) const noexcept
        {
            return _id == other._id;
        }
        bool operator!=(const Entity &other) const noexcept
        {
            return _id != other._id;
        }

    private:
        std::size_t _id;
    };
}
