#pragma once

#include <optional>
#include <vector>

namespace ecs {

template <typename Component>
class sparse_array {
public:
    using value_type = std::optional<Component>;
    using container_type = std::vector<value_type>;
    using size_type = typename container_type::size_type;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    sparse_array() = default;
    sparse_array(const sparse_array&) = default;
    sparse_array(sparse_array&&) noexcept = default;
    sparse_array& operator=(const sparse_array&) = default;
    sparse_array& operator=(sparse_array&&) noexcept = default;
    ~sparse_array() = default;

    // Element access
    value_type& operator[](size_type pos) {
        if (pos >= _data.size()) {
            _data.resize(pos + 1);
        }
        return _data[pos];
    }

    const value_type& operator[](size_type pos) const {
        static const value_type empty = std::nullopt;
        if (pos >= _data.size()) {
            return empty;
        }
        return _data[pos];
    }

    // Insert at position
    value_type& insert_at(size_type pos, const Component& value) {
        if (pos >= _data.size()) {
            _data.resize(pos + 1);
        }
        _data[pos] = value;
        return _data[pos];
    }

    value_type& insert_at(size_type pos, Component&& value) {
        if (pos >= _data.size()) {
            _data.resize(pos + 1);
        }
        _data[pos] = std::move(value);
        return _data[pos];
    }

    // Emplace at position
    template <typename... Args>
    value_type& emplace_at(size_type pos, Args&&... args) {
        if (pos >= _data.size()) {
            _data.resize(pos + 1);
        }
        _data[pos].emplace(std::forward<Args>(args)...);
        return _data[pos];
    }

    // Erase
    void erase(size_type pos) {
        if (pos < _data.size()) {
            _data[pos].reset();
        }
    }

    // Capacity
    size_type size() const noexcept {
        return _data.size();
    }

    bool empty() const noexcept {
        return _data.empty();
    }

    // Iterators
    iterator begin() noexcept {
        return _data.begin();
    }

    const_iterator begin() const noexcept {
        return _data.begin();
    }

    const_iterator cbegin() const noexcept {
        return _data.cbegin();
    }

    iterator end() noexcept {
        return _data.end();
    }

    const_iterator end() const noexcept {
        return _data.end();
    }

    const_iterator cend() const noexcept {
        return _data.cend();
    }

private:
    container_type _data;
};

} // namespace ecs
