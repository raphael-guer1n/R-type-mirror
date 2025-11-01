#pragma once
#include <tuple>
#include <optional>
#include <cstddef>

/**
 * @file Zipper_iterator.hpp
 * @brief Defines the engine::zipper_iterator class template for iterating over multiple containers in parallel.
 *
 * @namespace engine
 * @class zipper_iterator
 * @tparam Containers Variadic template parameter pack representing the container types to be zipped.
 *
 * @brief The zipper_iterator class template allows parallel iteration over multiple containers, 
 *        providing access to corresponding elements from each container as a tuple of references.
 *        It skips over indices where any container does not have a valid value (i.e., where std::optional is not set).
 *
 * @details
 * - The iterator is constructed with pointers to the containers, a starting index, and a maximum index.
 * - At each valid position, dereferencing the iterator yields a tuple of references to the values in each container.
 * - The iterator automatically skips indices where any container's element is not set (i.e., has_value() is false).
 * - The containers are expected to store elements of type std::optional<T>, and the value_type is a tuple of references to the contained values.
 *
 * @note
 * - The containers must support operator[] and return std::optional for their elements.
 * - The iterator is not a standard STL iterator, but provides similar semantics for use in range-based loops or manual iteration.
 *
 * @example
 * @code
 * std::vector<std::optional<int>> a = {1, std::nullopt, 3};
 * std::vector<std::optional<float>> b = {2.0f, 4.0f, std::nullopt};
 * engine::zipper_iterator<std::vector<std::optional<int>>, std::vector<std::optional<float>>> it({&a, &b}, 0, 3);
 * while (it != end) {
 *     auto [ai, bi] = *it;
 *     // Use ai and bi
 *     ++it;
 * }
 * @endcode
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
