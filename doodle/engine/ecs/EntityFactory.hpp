#pragma once
#include "engine/ecs/Registry.hpp"
#include <type_traits>
#include <utility>
#include <stdexcept>
/**
 * @file EntityFactory.hpp
 * @brief Defines utilities for creating entities with components in the ECS engine.
 */

namespace engine
{
    /**
     * @brief Creates a new entity and adds the specified components to it.
     *
     * Ensures that the registry is prepared to store each component type, then spawns a new entity
     * and adds the provided components to it.
     *
     * @tparam Components Variadic list of component types to add to the entity.
     * @param r Reference to the registry.
     * @param comps Instances of components to add to the entity.
     * @return entity_t The newly created entity.
     *
     * @throws std::bad_any_cast If a component array exists but is of the wrong type.
     */
    namespace detail
    {
        template <typename Component>
        void ensure_component_array(registry &r)
        {
            try {
                (void)r.get_components<Component>();
            }
            catch (std::bad_any_cast const &) {
                throw;
            }
            catch (std::out_of_range const &) {
                r.register_component<Component>();
            }
        }
    }

    template <typename... Components>
    entity_t make_entity(registry &r, Components &&...comps)
    {
        (detail::ensure_component_array<std::decay_t<Components>>(r), ...);
        auto entity = r.spawn_entity();
        (r.add_component<std::decay_t<Components>>(entity, std::forward<Components>(comps)), ...);
        return entity;
    }

} // namespace engine
