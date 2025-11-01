#pragma once
#include "Zipper_iterator.hpp"
#include <tuple>
#include <utility>
/**
    * @file Indexed_zipper_iterator.hpp
    * @brief An iterator that zips multiple containers together and includes the index.
    */  
namespace engine
{
    template <class... Containers>
    class indexed_zipper_iterator : public zipper_iterator<Containers...>
    {
        using base = zipper_iterator<Containers...>;

    public:
        using value_type = decltype(std::tuple_cat(
            std::make_tuple(std::size_t{}),
            std::declval<typename base::value_type>()));

        indexed_zipper_iterator(std::tuple<Containers *...> containers,
                                std::size_t idx,
                                std::size_t max)
            : base(containers, idx, max) {}

        value_type operator*()
        {
            auto comps = base::operator*();
            return std::tuple_cat(std::make_tuple(this->_index), comps);
        }
    };

}
