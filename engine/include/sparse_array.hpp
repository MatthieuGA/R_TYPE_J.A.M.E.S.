#pragma once
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace Engine {

template <typename Component>
class sparse_array {
 public:
    using value_type = std::optional<Component>;
    using reference_type = value_type &;
    using const_reference_type = value_type const &;
    using container_t = std::vector<value_type>;
    using size_type = typename container_t::size_type;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

 public:
    sparse_array();
    sparse_array(sparse_array const &);      // copy constructor
    sparse_array(sparse_array &&) noexcept;  // move constructor
    ~sparse_array();

    sparse_array &operator=(sparse_array const &);  // copy assignment operator
    sparse_array &operator=(sparse_array &&) noexcept;  // move assignment
    reference_type operator[](size_t idx);
    const_reference_type operator[](size_t idx) const;

    iterator begin();
    const_iterator begin() const;
    const_iterator cbegin() const;
    iterator end();
    const_iterator end() const;
    const_iterator cend() const;

    size_type size() const;
    bool has(size_type idx) const;
    reference_type insert_at(size_type pos, Component &&);
    reference_type insert_at(size_type pos, Component const &);

    template <class... Params>
    reference_type emplace_at(size_type pos, Params &&...);
    void erase(size_type pos);
    size_type get_index(value_type const &) const;

 private:
    container_t data_;
};

}  // namespace Engine

#include "sparse_array.tpp"
