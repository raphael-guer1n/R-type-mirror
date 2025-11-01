/**
 * @class engine::zipper
 * @brief Utility class to iterate over multiple containers in parallel (zip).
 *
 * The `zipper` class allows simultaneous iteration over several containers,
 * providing a zipped view where each iteration yields elements from all containers.
 * The iteration stops at the shortest container's end.
 *
 * @tparam Containers Variadic template parameter pack for container types to zip.
 *
 * @note The containers must support `.size()` and be compatible with the zipper_iterator.
 *
 * @see engine::zipper_iterator
 */
#pragma once
#include "Zipper_iterator.hpp"

namespace engine
{
    template <class... Containers>
    class zipper
    {
    public:
        using iterator = zipper_iterator<Containers...>;

        zipper(Containers &...cs) : _containers(&cs...)
        {
            _max = std::min({cs.size()...});
        }

        iterator begin() { return {_containers, 0, _max}; }
        iterator end() { return {_containers, _max, _max}; }

    private:
        std::tuple<Containers *...> _containers;
        std::size_t _max;
    };
}