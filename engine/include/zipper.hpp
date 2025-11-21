#pragma once
#include <tuple>
#include <cstddef>
#include <utility>
#include <optional>

namespace Engine {

template<class... Containers> class zipper;

template<class... Containers>
class zipper_iterator {
public:
    using iterator_tuple = std::tuple<Containers*...>;

    template<class Container>
    using elem_t = decltype(std::declval<Container>()[std::declval<std::size_t>()]);

    template<class T>
    using value_ref_t = decltype(std::declval<T>().value());

    template<class Container>
    using value_t = value_ref_t<elem_t<Container>>;

    using value_type = std::tuple<value_t<Containers>...>;

    using reference = value_type;
    using pointer = void;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    zipper_iterator() = default;

    friend class zipper<Containers...>;

    template<class...> friend class indexed_zipper_iterator;

private:
    zipper_iterator(iterator_tuple containers, std::size_t max, std::size_t idx = 0);

public:
    zipper_iterator& operator++();
    zipper_iterator operator++(int);
    value_type operator*() const;

    friend bool operator==(zipper_iterator const& a, zipper_iterator const& b) {
        return a._idx == b._idx && a._max == b._max && a._containers == b._containers;
    }
    friend bool operator!=(zipper_iterator const& a, zipper_iterator const& b) { return !(a==b); }

private:
    template<std::size_t... Is>
    bool all_set(std::index_sequence<Is...>) const;

    void skip_to_valid();

    template<std::size_t... Is>
    value_type to_value(std::index_sequence<Is...>) const;

    iterator_tuple _containers;
    std::size_t _max = 0;
    std::size_t _idx = 0;
    static constexpr auto _seq = std::index_sequence_for<Containers...>{};
};


template <class... Containers>
class zipper {
public:
    using iterator = zipper_iterator<Containers...>;

    explicit zipper(Containers&... cs);

    iterator begin() const;
    iterator end() const;

private:
    typename iterator::iterator_tuple _containers;
    std::size_t _size;
};

// Factory helper for easier deduction
template <class... Containers>
zipper<Containers...> make_zipper(Containers&... cs);

}  // namespace Engine

#include "zipper.tpp"
