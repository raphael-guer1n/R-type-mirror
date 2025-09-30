#pragma once
#include "Zipper_iterator.hpp"

namespace R_Ecs
{
    template <class... Containers>
    class Zipper
    {
    public:
        using iterator = Zipper_iterator<Containers...>;

        Zipper(Containers &...cs) : _containers(&cs...)
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