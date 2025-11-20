#pragma once
#include <vector>
#include <memory>
#include <utility>
#include <tuple>
#include <type_traits>
#include <algorithm>
#include <cstddef>
#include <iterator>

namespace Engine {
template <typename Component>
class sparse_array {
 public :
    using value_type = Component;
    using reference_type = value_type &;
    using const_reference_type = value_type const &;
    using container_t = std::vector<value_type>;
    using size_type = typename container_t::size_type;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

 public :
    sparse_array();
    sparse_array(sparse_array const &);  // copy constructor
    sparse_array(sparse_array &&) noexcept;  // move constructor
    ~sparse_array();

    sparse_array &operator=(sparse_array const &);  // copy assignment operator
    sparse_array &operator=(sparse_array &&) noexcept;  // move assignment
    reference_type operator[](size_t idx);
    const_reference_type operator[](size_t idx) const;

    iterator begin();
    const_iterator begin()const;
    const_iterator cbegin()const;
    iterator end();
    const_iterator end()const;
    const_iterator cend()const;

    size_type size()const;
bool has(size_type idx) const;
    reference_type insert_at(size_type pos, Component &&);
    reference_type insert_at(size_type pos, Component const&);

    template <class... Params>
    reference_type emplace_at(size_type pos, Params &&...);
    void erase(size_type pos);
    size_type get_index(value_type const &) const;

 private :
    container_t _data;
    std::vector<char> _present;
};

#pragma region Constructors / Destructors

template <typename Component>
sparse_array<Component>::sparse_array() : _data(), _present() {}


template <typename Component>
sparse_array<Component>::sparse_array(sparse_array const &other) :
_data(other._data), _present(other._present) {}

template <typename Component>
sparse_array<Component>::sparse_array(sparse_array &&other) noexcept :
_data(std::move(other._data)), _present(std::move(other._present)) {}

template <typename Component>
sparse_array<Component>::~sparse_array() = default;

#pragma endregion
#pragma region Assignment Operators

template <typename Component>
sparse_array<Component> &sparse_array<Component>::
operator=(sparse_array const &other) {
    if (this != &other)
        _data = other._data, _present = other._present;
    return *this;
}

template <typename Component>
sparse_array<Component> &sparse_array<Component>::
operator=(sparse_array &&other) noexcept {
    if (this != &other)
        _data = std::move(other._data), _present = std::move(other._present);
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
    return idx < _data.size() && idx < _present.size() &&
        static_cast<bool>(_present[idx]);
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
        _present.resize(_data.size());
    }
    _data[pos] = Component(std::forward<Params>(params)...);
    _present[pos] = 1;
    return _data[pos];
}

template <typename Component>
void sparse_array<Component>::erase(size_type pos) {
    if (pos >= _data.size())
        return;
    _data[pos] = Component();
    _present[pos] = 0;
}

template <typename Component>
typename sparse_array<Component>::size_type sparse_array<Component>::
get_index(value_type const &val) const {
    for (size_type i = 0; i < _data.size(); ++i) {
        if (std::addressof(_data[i]) == std::addressof(val)) {
            return i;
        }
    }
    return static_cast<size_type>(-1);
}

#pragma endregion
#pragma region Zipper

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
    std::size_t idx = 0)
        : _containers(containers), _max(max), _idx(idx) {}

    zipper_iterator(zipper_iterator const &z) = default;

    // pre-increment
    zipper_iterator &operator++() {
        ++_idx;
        skip_to_valid();
        return *this;
    }

    // post-increment
    zipper_iterator operator++(int) {
        zipper_iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    value_type operator*() const {
        return to_value(_seq);
    }

    value_type operator->() const {
        return operator*();
    }

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
    void incr_all(std::index_sequence<Is...>) {
        (void)std::initializer_list<int>{
            ((void)++std::get<Is>(_containers), 0)...
        };
    }

    template <std::size_t... Is>
    bool all_set(std::index_sequence<Is...>) const {
        bool ok = true;
        (void)std::initializer_list<int>{
            ((ok = ok && std::get<Is>(_containers)->has(_idx)), 0)...
        };
        return ok;
    }

    template <std::size_t... Is>
    value_type to_value(std::index_sequence<Is...>) const {
        return std::forward_as_tuple(
            (std::get<Is>(_containers)->operator[](_idx))...);
    }

      void skip_to_valid() {
        while (_idx < _max) {
            if (all_set(_seq)) break;
            ++_idx;
        }
    }

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
    explicit zipper(Containers const &... cs) : _begin(make_tuple_ptrs(cs...)),
        _size(_compute_size(cs...)) {}

    iterator begin() {
        iterator it(_begin, _size, 0); it.skip_to_valid();
        return it;
    }
    iterator end() { return iterator(_begin, _size, _size); }

 public:
    template <std::size_t... Is>
    static std::size_t _compute_size_impl(std::index_sequence<Is...>,
    Containers const &... cs) {
        std::size_t vals[] = { cs.size()... };
        return *std::max_element(std::begin(vals), std::end(vals));
    }

    static std::size_t _compute_size(Containers const &... cs) {
        return _compute_size_impl(std::index_sequence_for<Containers...>{},
            cs...);
    }

    template <class... C>
    static iterator_tuple make_tuple_ptrs(C const &... cs) {
        return iterator_tuple(&cs...);
    }

    template <class... C>
    static iterator_tuple _get_ptrs(C const &... cs) {
        return iterator_tuple(&cs...);
    }

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
        std::size_t max, std::size_t idx = 0)
        : base(containers, max, idx) {}

    value_type operator*() const {
        return std::tuple_cat(
            std::tuple<std::size_t>(base::_idx), base::operator*());
    }
};

template <class... Containers>
class indexed_zipper {
 public:
    using iterator = indexed_zipper_iterator<Containers...>;
    explicit indexed_zipper(Containers const &... cs) : _zip(cs...) {}
    iterator begin() {
        auto it = iterator(_zip._begin, _zip._size, 0);
        it.skip_to_valid(); return it;
    }
    iterator end() {
        return iterator(_zip._begin, _zip._size, _zip._size);
    }

 private:
    zipper<Containers...> _zip;
};

// Factory helpers for easier deduction
template <class... Containers>
zipper<Containers...> make_zipper(Containers const &... cs) {
    return zipper<Containers...>(cs...);
}

template <class... Containers>
indexed_zipper<Containers...> make_indexed_zipper(Containers const &... cs) {
    return indexed_zipper<Containers...>(cs...);
}

#pragma endregion
}  // namespace Engine
