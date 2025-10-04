#pragma once
#include <vector>
#include <optional>
#include <cstddef>
#include <memory>
#include <algorithm>
/**
 * @file Sparse_array.hpp
 * @brief Defines the engine::sparse_array template class for efficient sparse storage of components.
 *
 * The sparse_array class provides a container for storing elements (typically ECS components)
 * in a sparse manner, using std::optional to represent the presence or absence of a component
 * at a given index. It supports random access, insertion, emplacing, erasure, and iteration.
 * This is particularly useful in Entity-Component-System (ECS) architectures where not all
 * entities have all components.
 *
 * @tparam Component The type of component to store in the sparse array.
 */
namespace engine
{
    template <typename Component>
    class sparse_array
    {
    public:
        using value_type = std::optional<Component>;
        using reference_type = value_type &;
        using const_reference_type = value_type const &;
        using container_t = std::vector<value_type>;
        using size_type = typename container_t::size_type;
        using iterator = typename container_t::iterator;
        using const_iterator = typename container_t::const_iterator;

    public:
        // Constructors
        sparse_array() = default;
        sparse_array(sparse_array const &) = default;
        sparse_array(sparse_array &&) noexcept = default;
        sparse_array &operator=(sparse_array const &) = default;
        sparse_array &operator=(sparse_array &&) noexcept = default;
        ~sparse_array() = default;

        // Element access
        reference_type operator[](size_t idx) { return _data[idx]; }
        const_reference_type operator[](size_t idx) const { return _data[idx]; }

        // Iterators
        iterator begin() { return _data.begin(); }
        const_iterator begin() const { return _data.begin(); }
        const_iterator cbegin() const { return _data.cbegin(); }

        iterator end() { return _data.end(); }
        const_iterator end() const { return _data.end(); }
        const_iterator cend() const { return _data.cend(); }

        // Capacity
        size_type size() const { return _data.size(); }

        // Modifiers
        reference_type insert_at(size_type pos, Component const &c)
        {
            if (pos >= _data.size())
                _data.resize(pos + 1);
            _data[pos] = c;
            return _data[pos];
        }

        reference_type insert_at(size_type pos, Component &&c)
        {
            if (pos >= _data.size())
                _data.resize(pos + 1);
            _data[pos] = std::move(c);
            return _data[pos];
        }

        template <class... Params>
        reference_type emplace_at(size_type pos, Params &&...params)
        {
            if (pos >= _data.size())
                _data.resize(pos + 1);
            _data[pos].emplace(std::forward<Params>(params)...);
            return _data[pos];
        }

        void erase(size_type pos)
        {
            if (pos < _data.size())
                _data[pos] = std::nullopt;
        }

        size_type get_index(value_type const &opt) const
        {
            auto it = std::find_if(_data.begin(), _data.end(), [&](auto const &v)
                                   { return &v == &opt; });
            return it != _data.end() ? std::distance(_data.begin(), it) : static_cast<size_type>(-1);
        }

    private:
        container_t _data;
    };

}