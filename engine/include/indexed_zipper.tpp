#pragma once
#include "indexed_zipper.hpp"

namespace Engine {

// indexed_zipper_iterator implementation
template<class... Containers>
indexed_zipper_iterator<Containers...>::indexed_zipper_iterator(base_iterator it)
    : _it(it), _idx(_it._idx) {}

template<class... Containers>
indexed_zipper_iterator<Containers...>& indexed_zipper_iterator<Containers...>::operator++() {
    ++_it;
    _idx = _it._idx;
    return *this;
}

template<class... Containers>
indexed_zipper_iterator<Containers...> indexed_zipper_iterator<Containers...>::operator++(int) {
    indexed_zipper_iterator tmp = *this;
    ++(*this);
    return tmp;
}

template<class... Containers>
typename indexed_zipper_iterator<Containers...>::value_type
indexed_zipper_iterator<Containers...>::operator*() const {
    return std::tuple_cat(std::tuple<std::size_t>(_idx), *_it);
}

// indexed_zipper implementation
template <class... Containers>
indexed_zipper<Containers...>::indexed_zipper(Containers&... cs) : _zip(cs...) {}

template <class... Containers>
typename indexed_zipper<Containers...>::iterator
indexed_zipper<Containers...>::begin() const {
    return iterator(_zip.begin());
}

template <class... Containers>
typename indexed_zipper<Containers...>::iterator
indexed_zipper<Containers...>::end() const {
    return iterator(_zip.end());
}

// Factory helper
template <class... Containers>
indexed_zipper<Containers...> make_indexed_zipper(Containers&... cs) {
    return indexed_zipper<Containers...>(cs...);
}

}  // namespace Engine
