#pragma once
#include "Indexed_zipper_iterator.hpp"
#include <tuple>
#include <algorithm>

namespace engine
{
    template <class... Containers>
    class indexed_zipper
    {
    public:
        using iterator = indexed_zipper_iterator<Containers...>;

        indexed_zipper(Containers &...cs) : _containers(&cs...)
        {
            _max = std::min({cs.size()...});
        }

        iterator begin() { return iterator(_containers, 0, _max); }
        iterator end() { return iterator(_containers, _max, _max); }

    private:
        std::tuple<Containers *...> _containers;
        std::size_t _max;
    };

}
