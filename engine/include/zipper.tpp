#pragma once
#include "zipper.hpp"
#include <algorithm>
#include <iterator>

namespace Engine {

// zipper_iterator implementations
template <class... Containers>
zipper_iterator<Containers...>::zipper_iterator(
    iterator_tuple const &containers, std::size_t max, std::size_t idx)
    : _containers(containers), _max(max), _idx(idx) {}

template <class... Containers>
zipper_iterator<Containers...> &zipper_iterator<Containers...>::operator++() {
    ++_idx;
    skip_to_valid();
    return *this;
}

template <class... Containers>
zipper_iterator<Containers...> zipper_iterator<Containers...>::operator++(int) {
    zipper_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template <class... Containers>
typename zipper_iterator<Containers...>::value_type
zipper_iterator<Containers...>::operator*() const {
    return to_value(_seq);
}

template <class... Containers>
typename zipper_iterator<Containers...>::value_type
zipper_iterator<Containers...>::operator->() const {
    return operator*();
}

template <class... Containers>
template <std::size_t... Is>
void zipper_iterator<Containers...>::incr_all(std::index_sequence<Is...>) {
    (void)std::initializer_list<int>{
        ((void)++std::get<Is>(_containers), 0)...
    };
}

template <class... Containers>
template <std::size_t... Is>
bool zipper_iterator<Containers...>::all_set(std::index_sequence<Is...>) const {
    bool ok = true;
    (void)std::initializer_list<int>{
        ((ok = ok && std::get<Is>(_containers)->has(_idx)), 0)...
    };
    return ok;
}

template <class... Containers>
template <std::size_t... Is>
typename zipper_iterator<Containers...>::value_type
zipper_iterator<Containers...>::to_value(std::index_sequence<Is...>) const {
    return std::forward_as_tuple(
        (std::get<Is>(_containers)->operator[](_idx))...);
}

template <class... Containers>
void zipper_iterator<Containers...>::skip_to_valid() {
    while (_idx < _max) {
        if (all_set(_seq)) break;
        ++_idx;
    }
}

// zipper implementations
template <class... Containers>
zipper<Containers...>::zipper(Containers const &... cs)
    : _begin(make_tuple_ptrs(cs...)), _size(_compute_size(cs...)) {}

template <class... Containers>
typename zipper<Containers...>::iterator zipper<Containers...>::begin() {
    iterator it(_begin, _size, 0);
    it.skip_to_valid();
    return it;
}

template <class... Containers>
typename zipper<Containers...>::iterator zipper<Containers...>::end() {
    return iterator(_begin, _size, _size);
}

template <class... Containers>
template <std::size_t... Is>
std::size_t zipper<Containers...>::_compute_size_impl(
    std::index_sequence<Is...>, Containers const &... cs) {
    std::size_t vals[] = { cs.size()... };
    return *std::max_element(std::begin(vals), std::end(vals));
}

template <class... Containers>
std::size_t zipper<Containers...>::_compute_size(Containers const &... cs) {
    return _compute_size_impl(std::index_sequence_for<Containers...>{}, cs...);
}

template <class... Containers>
template <class... C>
typename zipper<Containers...>::iterator_tuple
zipper<Containers...>::make_tuple_ptrs(C const &... cs) {
    return iterator_tuple(&cs...);
}

template <class... Containers>
template <class... C>
typename zipper<Containers...>::iterator_tuple
zipper<Containers...>::_get_ptrs(C const &... cs) {
    return iterator_tuple(&cs...);
}

// indexed_zipper_iterator implementations
template <class... Containers>
indexed_zipper_iterator<Containers...>::indexed_zipper_iterator(
    typename base::iterator_tuple const &containers,
    std::size_t max, std::size_t idx)
    : base(containers, max, idx) {}

template <class... Containers>
typename indexed_zipper_iterator<Containers...>::value_type
indexed_zipper_iterator<Containers...>::operator*() const {
    return std::tuple_cat(
        std::tuple<std::size_t>(base::_idx), base::operator*());
}

// indexed_zipper implementations
template <class... Containers>
indexed_zipper<Containers...>::indexed_zipper(Containers const &... cs)
    : _zip(cs...) {}

template <class... Containers>
typename indexed_zipper<Containers...>::iterator
indexed_zipper<Containers...>::begin() {
    auto it = iterator(_zip._begin, _zip._size, 0);
    it.skip_to_valid();
    return it;
}

template <class... Containers>
typename indexed_zipper<Containers...>::iterator
indexed_zipper<Containers...>::end() {
    return iterator(_zip._begin, _zip._size, _zip._size);
}

// Factory helpers
template <class... Containers>
zipper<Containers...> make_zipper(Containers const &... cs) {
    return zipper<Containers...>(cs...);
}

template <class... Containers>
indexed_zipper<Containers...> make_indexed_zipper(Containers const &... cs) {
    return indexed_zipper<Containers...>(cs...);
}

}  // namespace Engine
