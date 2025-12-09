#pragma once

#include <cstddef>
#include <tuple>

#include "include/zipper.hpp"

namespace Engine {

// Forward declaration
template <class... Containers>
class indexed_zipper;

// indexed_zipper_iterator: same as zipper_iterator but also returns the index
template <class... Containers>
class indexed_zipper_iterator {
 public:
    using base_iterator = zipper_iterator<Containers...>;
    using value_type = decltype(std::tuple_cat(std::tuple<std::size_t>{},
        std::declval<typename base_iterator::value_type>()));

    using reference = value_type;
    using pointer = void;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    indexed_zipper_iterator() = default;

    friend class indexed_zipper<Containers...>;

 private:
    explicit indexed_zipper_iterator(base_iterator it);

 public:
    indexed_zipper_iterator &operator++();
    indexed_zipper_iterator operator++(int);
    value_type operator*() const;

    friend bool operator==(
        indexed_zipper_iterator const &a, indexed_zipper_iterator const &b) {
        return a._it == b._it;
    }

    friend bool operator!=(
        indexed_zipper_iterator const &a, indexed_zipper_iterator const &b) {
        return !(a == b);
    }

 private:
    base_iterator _it;
    std::size_t _idx = 0;
};

template <class... Containers>
class indexed_zipper {
 public:
    using iterator = indexed_zipper_iterator<Containers...>;

    explicit indexed_zipper(Containers &...cs);

    iterator begin() const;
    iterator end() const;

 private:
    zipper<Containers...> _zip;
};

// Factory helper for easier deduction
template <class... Containers>
indexed_zipper<Containers...> make_indexed_zipper(Containers &...cs);

}  // namespace Engine

#include "indexed_zipper.tpp"
