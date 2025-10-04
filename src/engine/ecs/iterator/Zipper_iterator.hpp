#pragma once
#include <tuple>
#include <optional>
#include <cstddef>
/**
    * @file Zipper_iterator.hpp
    * @brief An iterator that zips multiple containers together.
    */
namespace engine
{
    template <class... Containers>
    class zipper_iterator
    {
        template <class Container>
        using ref_t = typename Container::value_type::value_type &;

    public:
        using value_type = std::tuple<ref_t<Containers>...>;

        zipper_iterator(std::tuple<Containers *...> containers, std::size_t idx, std::size_t max)
            : _containers(containers), _index(idx), _max(max)
        {
            skip_invalid();
        }

        value_type operator*() { return to_value(std::index_sequence_for<Containers...>{}); }

        zipper_iterator &operator++()
        {
            _index++;
            skip_invalid();
            return *this;
        }

        bool operator!=(const zipper_iterator &other) const { return _index != other._index; }

    private:
        template <std::size_t... Is>
        value_type to_value(std::index_sequence<Is...>)
        {
            return {std::get<Is>(_containers)->operator[](_index).value()...};
        }

        template <std::size_t... Is>
        bool all_set(std::index_sequence<Is...>)
        {
            return (... && std::get<Is>(_containers)->operator[](_index).has_value());
        }

        void skip_invalid()
        {
            while (_index < _max && !all_set(std::index_sequence_for<Containers...>{}))
            {
                _index++;
            }
        }

    protected:
        std::tuple<Containers *...> _containers;
        std::size_t _index;
        std::size_t _max;
    };
}
