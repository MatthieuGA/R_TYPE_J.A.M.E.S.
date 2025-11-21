#pragma once
#include <tuple>
#include <cstddef>
#include <utility>

namespace Engine {

// zipper: iterate several sparse_array-like containers simultaneously,
// yielding tuples of references to elements where all containers have a value.
template <class... Containers>
class zipper_iterator {
    template <class Container>
    using elem_t = decltype(std::declval<Container const>()
        [std::declval<std::size_t>()]);

 public:
    using value_type = std::tuple<elem_t<Containers>...>;
    using reference = value_type;
    using pointer = void;
    using difference_type = std::size_t;
    using iterator_category = std::input_iterator_tag;
    using iterator_tuple = std::tuple<Containers const*...>;

    zipper_iterator() = default;
    zipper_iterator(iterator_tuple const &containers, std::size_t max,
        std::size_t idx = 0);
    zipper_iterator(zipper_iterator const &z) = default;

    // pre-increment
    zipper_iterator &operator++();
    // post-increment
    zipper_iterator operator++(int);

    value_type operator*() const;
    value_type operator->() const;

    friend bool operator==(zipper_iterator const &lhs,
        zipper_iterator const &rhs) {
        return lhs._idx == rhs._idx;
    }

    friend bool operator!=(zipper_iterator const &lhs,
        zipper_iterator const &rhs) {
        return !(lhs == rhs);
    }

 public:
    template <std::size_t... Is>
    void incr_all(std::index_sequence<Is...>);

    template <std::size_t... Is>
    bool all_set(std::index_sequence<Is...>) const;

    template <std::size_t... Is>
    value_type to_value(std::index_sequence<Is...>) const;

    void skip_to_valid();

    iterator_tuple _containers;
    std::size_t _max = 0;
    std::size_t _idx = 0;
    static constexpr std::index_sequence_for<Containers...> _seq{};
};

template <class... Containers>
class zipper {
 public:
    using iterator = zipper_iterator<Containers...>;
    using iterator_tuple = typename iterator::iterator_tuple;

    explicit zipper(Containers const &... cs);

    iterator begin();
    iterator end();

 public:
    template <std::size_t... Is>
    static std::size_t _compute_size_impl(std::index_sequence<Is...>,
        Containers const &... cs);

    static std::size_t _compute_size(Containers const &... cs);

    template <class... C>
    static iterator_tuple make_tuple_ptrs(C const &... cs);

    template <class... C>
    static iterator_tuple _get_ptrs(C const &... cs);

 public:
    iterator_tuple _begin;
    std::size_t _size;
};

// indexed_zipper: same as zipper but returns tuple<size_t, refs...>
template <class... Containers>
class indexed_zipper_iterator : public zipper_iterator<Containers...> {
    using base = zipper_iterator<Containers...>;
 public:
    using value_type = decltype(std::tuple_cat(std::tuple<std::size_t>{},
        std::declval<typename base::value_type>()));

    indexed_zipper_iterator(typename base::iterator_tuple const &containers,
        std::size_t max, std::size_t idx = 0);

    value_type operator*() const;
};

template <class... Containers>
class indexed_zipper {
 public:
    using iterator = indexed_zipper_iterator<Containers...>;

    explicit indexed_zipper(Containers const &... cs);
    iterator begin();
    iterator end();

 private:
    zipper<Containers...> _zip;
};

// Factory helpers for easier deduction
template <class... Containers>
zipper<Containers...> make_zipper(Containers const &... cs);

template <class... Containers>
indexed_zipper<Containers...> make_indexed_zipper(Containers const &... cs);

}  // namespace Engine

#include "zipper.tpp"
