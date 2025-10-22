#pragma once
#include <cstddef>
/**    * @file Entity.hpp
    * @brief A simple wrapper for entity identifiers in the ECS.
    */
    
namespace engine
{
    class entity_t
    /**
     * @class entity_t
     * @brief Represents a unique entity identifier in the ECS (Entity Component System).
     *
     * This class encapsulates an entity's unique identifier, providing type safety and
     * convenient comparison and conversion operations.
     */

    /**
     * @brief Constructs an entity_t from a given std::size_t identifier.
     * @param id The unique identifier for the entity.
     */

    /**
     * @brief Converts the entity_t to its underlying std::size_t identifier.
     * @return The unique identifier of the entity.
     */

    /**
     * @brief Checks if two entity_t instances represent the same entity.
     * @param other The entity_t to compare with.
     * @return True if both entities have the same identifier, false otherwise.
     */

    /**
     * @brief Checks if two entity_t instances represent different entities.
     * @param other The entity_t to compare with.
     * @return True if the entities have different identifiers, false otherwise.
     */
    {
        /** @brief Construct an entity_t from a std::size_t */
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
