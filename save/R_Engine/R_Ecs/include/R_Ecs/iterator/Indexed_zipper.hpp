#pragma once
#include "Indexed_zipper_iterator.hpp"
#include <tuple>
#include <algorithm>

namespace R_Ecs
{
    template <class... Containers>
    class Indexed_zipper
    {
    public:
        using iterator = Indexed_zipper_iterator<Containers...>;

        Indexed_zipper(Containers &...cs) : _containers(&cs...)
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
