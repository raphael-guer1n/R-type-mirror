#pragma once

#include <unordered_map>
#include <typeindex>
#include <typeinfo>
#include <any>
#include <functional>
#include <vector>

#include "Entity.hpp"
#include "Sparse_array.hpp"

namespace engine
{
    class registry
    {
    public:
        registry() = default;
        ~registry() = default;

        template <class Component>
        sparse_array<Component> &register_component()
        {
            std::type_index ti(typeid(Component));
            if (_components_arrays.find(ti) == _components_arrays.end())
            {
                _components_arrays[ti] = sparse_array<Component>();
                _erase_funcs[ti] = [](registry &r, entity_t const &e)
                {
                    auto &arr = r.get_components<Component>();
                    arr.erase(static_cast<std::size_t>(e));
                };
            }
            return std::any_cast<sparse_array<Component> &>(_components_arrays[ti]);
        }

        template <class Component>
        sparse_array<Component> &get_components()
        {
            return std::any_cast<sparse_array<Component> &>(_components_arrays.at(std::type_index(typeid(Component))));
        }

        template <class Component>
        sparse_array<Component> const &get_components() const
        {
            return std::any_cast<sparse_array<Component> const &>(_components_arrays.at(std::type_index(typeid(Component))));
        }

        entity_t spawn_entity()
        {
            entity_t e(_next_entity_id++);
            _alive.push_back(static_cast<std::size_t>(e));
            return e;
        }

        void kill_entity(entity_t const &e)
        {
            auto it = std::find(_alive.begin(), _alive.end(), static_cast<std::size_t>(e));
            if (it != _alive.end())
            {
                _alive.erase(it);
                for (auto &[ti, erase_fn] : _erase_funcs)
                {
                    erase_fn(*this, e);
                }
            }
        }

        entity_t entity_from_index(std::size_t idx)
        {
            return entity_t(idx);
        }

        template <typename Component>
        typename sparse_array<Component>::reference_type
        add_component(entity_t const &e, Component &&c)
        {
            return get_components<Component>().insert_at(static_cast<std::size_t>(e), std::move(c));
        }

        template <typename Component, typename... Params>
        typename sparse_array<Component>::reference_type
        emplace_component(entity_t const &e, Params &&...params)
        {
            return get_components<Component>().emplace_at(static_cast<std::size_t>(e),
                                                          std::forward<Params>(params)...);
        }

        template <typename Component>
        void remove_component(entity_t const &e)
        {
            get_components<Component>().erase(static_cast<std::size_t>(e));
        }

        template <class... Components, typename Function>
        void add_system(Function &&f)
        {
            _systems.emplace_back(
                [func = std::forward<Function>(f)](registry &r)
                {
                    func(r, r.get_components<Components>()...);
                });
        }

        void run_systems()
        {
            for (auto &system : _systems)
                system(*this);
        }

    private:
        std::unordered_map<std::type_index, std::any> _components_arrays;
        std::unordered_map<std::type_index, std::function<void(registry &, entity_t const &)>> _erase_funcs;
        std::vector<std::size_t> _alive;
        std::size_t _next_entity_id{0};
        std::vector<std::function<void(registry &)>> _systems;
    };

}
