#pragma once
#include "engine/ecs/Registry.hpp"
#include <type_traits>
#include <utility>
#include <stdexcept>

namespace engine
{

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
