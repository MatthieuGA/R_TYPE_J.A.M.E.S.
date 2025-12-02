#pragma once
#include "zipper.hpp"
#include <algorithm>

namespace Engine {

// zipper_iterator implementation
template<class... Containers>
zipper_iterator<Containers...>::zipper_iterator(iterator_tuple containers, std::size_t max, std::size_t idx)
    : _containers(containers), _max(max), _idx(idx) {
    skip_to_valid();
}

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
template <std::size_t... Is>
bool zipper_iterator<Containers...>::all_set(std::index_sequence<Is...>) const {
    return ( (_idx < std::get<Is>(_containers)->size() && 
              std::get<Is>(_containers)->operator[](_idx).has_value()) && ... );
}

template <class... Containers>
void zipper_iterator<Containers...>::skip_to_valid() {
    while (_idx < _max && !all_set(_seq)) ++_idx;
}

template<class... Containers>
template<std::size_t... Is>
typename zipper_iterator<Containers...>::value_type
zipper_iterator<Containers...>::to_value(std::index_sequence<Is...>) const {
    return std::forward_as_tuple( (std::get<Is>(_containers)->operator[](_idx).value())... );
}


template <class... Containers>
zipper<Containers...>::zipper(Containers&... cs) : _containers(&cs...) {
    std::size_t sizes[] = { cs.size()... };
    _size = *std::max_element(std::begin(sizes), std::end(sizes));
}

template <class... Containers>
typename zipper<Containers...>::iterator
zipper<Containers...>::begin() const {
    return iterator(_containers, _size, 0);
}

template <class... Containers>
typename zipper<Containers...>::iterator
zipper<Containers...>::end() const {
    return iterator(_containers, _size, _size);
}

// Factory helper
template <class... Containers>
zipper<Containers...> make_zipper(Containers&... cs) {
    return zipper<Containers...>(cs...);
}

}  // namespace Engine
