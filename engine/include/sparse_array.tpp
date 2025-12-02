#pragma once
#include "sparse_array.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace Engine {

#pragma region Constructors / Destructors

template <typename Component>
sparse_array<Component>::sparse_array() : data_() {}

template <typename Component>
sparse_array<Component>::sparse_array(sparse_array const &other) :
data_(other.data_) {}

template <typename Component>
sparse_array<Component>::sparse_array(sparse_array &&other) noexcept :
data_(std::move(other.data_)) {}

template <typename Component>
sparse_array<Component>::~sparse_array() = default;

#pragma endregion
#pragma region Assignment Operators

template <typename Component>
sparse_array<Component> &sparse_array<Component>::
operator=(sparse_array const &other) {
    if (this != &other)
        data_ = other.data_;
    return *this;
}

template <typename Component>
sparse_array<Component> &sparse_array<Component>::
operator=(sparse_array &&other) noexcept {
    if (this != &other)
        data_ = std::move(other.data_);
    return *this;
}

#pragma endregion
#pragma region Element Access

template <typename Component>
typename sparse_array<Component>::reference_type sparse_array<Component>::
operator[](size_t idx) {
    return data_[idx];
}

template <typename Component>
typename sparse_array<Component>::const_reference_type sparse_array<Component>::
operator[](size_t idx) const {
    return data_[idx];
}

#pragma endregion
#pragma region Iterators

template <typename Component>
typename sparse_array<Component>::iterator sparse_array<Component>::
begin() {
    return data_.begin();
}

template <typename Component>
typename sparse_array<Component>::const_iterator sparse_array<Component>::
begin() const {
    return data_.begin();
}

template <typename Component>
typename sparse_array<Component>::const_iterator sparse_array<Component>::
cbegin() const {
    return data_.cbegin();
}

template <typename Component>
typename sparse_array<Component>::iterator sparse_array<Component>::
end() {
    return data_.end();
}

template <typename Component>
typename sparse_array<Component>::const_iterator sparse_array<Component>::
end() const {
    return data_.end();
}

template <typename Component>
typename sparse_array<Component>::const_iterator sparse_array<Component>::
cend() const {
    return data_.cend();
}

#pragma endregion
#pragma region Capacity

template <typename Component>
typename sparse_array<Component>::size_type sparse_array<Component>::
size() const {
    return data_.size();
}

template <typename Component>
bool sparse_array<Component>::has(size_type idx) const {
    return idx < data_.size() && data_[idx].has_value();
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
    if (pos >= data_.size()) {
        data_.resize(pos + 1);
    }
    data_[pos].emplace(std::forward<Params>(params)...);
    return data_[pos];
}

template <typename Component>
void sparse_array<Component>::erase(size_type pos) {
    if (pos >= data_.size())
        return;
    data_[pos].reset();
}

template <typename Component>
typename sparse_array<Component>::size_type sparse_array<Component>::
get_index(value_type const &value) const {
    const auto *ptr = &value;
    const auto *base = data_.data();

    if (ptr < base || ptr >= base + data_.size()) {
        return static_cast<size_type>(-1);
    }

    return static_cast<size_type>(ptr - base);
}

#pragma endregion

}  // namespace Engine
