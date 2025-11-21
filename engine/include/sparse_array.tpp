#pragma once
#include "sparse_array.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace Engine {

#pragma region Constructors / Destructors

template <typename Component>
sparse_array<Component>::sparse_array() : _data() {}

template <typename Component>
sparse_array<Component>::sparse_array(sparse_array const &other) :
_data(other._data) {}

template <typename Component>
sparse_array<Component>::sparse_array(sparse_array &&other) noexcept :
_data(std::move(other._data)) {}

template <typename Component>
sparse_array<Component>::~sparse_array() = default;

#pragma endregion
#pragma region Assignment Operators

template <typename Component>
sparse_array<Component> &sparse_array<Component>::
operator=(sparse_array const &other) {
    if (this != &other)
        _data = other._data;
    return *this;
}

template <typename Component>
sparse_array<Component> &sparse_array<Component>::
operator=(sparse_array &&other) noexcept {
    if (this != &other)
        _data = std::move(other._data);
    return *this;
}

#pragma endregion
#pragma region Element Access

template <typename Component>
typename sparse_array<Component>::reference_type sparse_array<Component>::
operator[](size_t idx) {
    return _data[idx];
}

template <typename Component>
typename sparse_array<Component>::const_reference_type sparse_array<Component>::
operator[](size_t idx) const {
    return _data[idx];
}

#pragma endregion
#pragma region Iterators

template <typename Component>
typename sparse_array<Component>::iterator sparse_array<Component>::
begin() {
    return _data.begin();
}

template <typename Component>
typename sparse_array<Component>::const_iterator sparse_array<Component>::
begin() const {
    return _data.begin();
}

template <typename Component>
typename sparse_array<Component>::const_iterator sparse_array<Component>::
cbegin() const {
    return _data.cbegin();
}

template <typename Component>
typename sparse_array<Component>::iterator sparse_array<Component>::
end() {
    return _data.end();
}

template <typename Component>
typename sparse_array<Component>::const_iterator sparse_array<Component>::
end() const {
    return _data.end();
}

template <typename Component>
typename sparse_array<Component>::const_iterator sparse_array<Component>::
cend() const {
    return _data.cend();
}

#pragma endregion
#pragma region Capacity

template <typename Component>
typename sparse_array<Component>::size_type sparse_array<Component>::
size() const {
    return _data.size();
}

template <typename Component>
bool sparse_array<Component>::has(size_type idx) const {
    return idx < _data.size() && _data[idx].has_value();
}

#pragma endregion
#pragma region Modifiers

template <typename Component>
typename sparse_array<Component>::reference_type sparse_array<Component>::
insert_at(size_type pos, Component &&comp) {
    return emplace_at(pos, std::move(comp));
}

template <typename Component>
typename sparse_array<Component>::reference_type sparse_array<Component>::
insert_at(size_type pos, Component const &comp) {
    return emplace_at(pos, comp);
}

template <typename Component>
template <class... Params>
typename sparse_array<Component>::reference_type sparse_array<Component>::
emplace_at(size_type pos, Params &&... params) {
    if (pos >= _data.size()) {
        _data.resize(pos + 1);
    }
    _data[pos].emplace(std::forward<Params>(params)...);
    return _data[pos];
}

template <typename Component>
void sparse_array<Component>::erase(size_type pos) {
    if (pos >= _data.size())
        return;
    _data[pos].reset();
}

template <typename Component>
typename sparse_array<Component>::size_type sparse_array<Component>::
get_index(value_type const &value) const {
    const auto *ptr = &value;
    const auto *base = _data.data();
    
    if (ptr < base || ptr >= base + _data.size()) {
        return static_cast<size_type>(-1);
    }
    
    return static_cast<size_type>(ptr - base);
}

#pragma endregion

}  // namespace Engine
