#pragma once
#include <unordered_map>
#include <typeindex>
#include <stdexcept>
#include <memory>
#include <functional>
#include <utility>
#include <vector>

#include "include/sparse_array.hpp"
#include "include/entity.hpp"

namespace Engine {
struct components_holder_base {
    virtual ~components_holder_base() = default;
};

template <typename T>
struct components_holder : components_holder_base {
    sparse_array<T> arr;
};

class registry {
 public :
    using entity_t = entity;
    template <class Component>
    sparse_array<Component> &register_component();
    template <class Component>
    sparse_array<Component> &get_components();
    template <class Component>
    sparse_array<Component> const &get_components() const;

    // Entity management
    entity_t spawn_entity();
    entity_t entity_from_index(std::size_t idx);
    void kill_entity(entity_t const &e);

    template <class ... Components, typename Function>
    void add_system(Function &&f);

    void run_systems();


    template <typename Component>
    typename sparse_array<Component>::reference_type
        add_component(entity_t const &to, Component &&c);

    template <typename Component, typename ... Params>
    typename sparse_array<Component>::reference_type
        emplace_component(entity_t const &to, Params &&... p);

    template <typename Component>
    void remove_component(entity_t const &from);

 private:
    std::unordered_map <std::type_index,
        std::unique_ptr<components_holder_base>> _components_arrays;
    std::vector<std::function<
        void(registry&, Engine::entity const&)>> _erase_fns;
    std::vector<std::function<void(registry&)>> _systems;
    std::size_t _next_entity = 0;
    std::vector<std::size_t> _dead_entities;
};

// Template definitions (must be in header for linkage)
template <class Component>
inline sparse_array<Component> &registry::register_component() {
    auto type_idx = std::type_index(typeid(Component));
    auto it = _components_arrays.find(type_idx);
    if (it == _components_arrays.end()) {
        auto holder = std::make_unique<components_holder<Component>>();
        _components_arrays[type_idx] = std::move(holder);
        std::function<void(registry&, entity const&)> fn =
            [](registry &r, entity const &e) {
        try {
            r.get_components<Component>().erase(e.getId());
        } catch (...) {}
        };
        _erase_fns.push_back(std::move(fn));
    } else {
        throw std::runtime_error("Component already registered in registry");
    }
    return static_cast<components_holder<Component>*>
        (_components_arrays[type_idx].get())->arr;
}

template <class Component>
inline sparse_array<Component> &registry::get_components() {
    auto type_idx = std::type_index(typeid(Component));
    auto it = _components_arrays.find(type_idx);
    if (it == _components_arrays.end()) {
        throw std::runtime_error("Component not registered in registry");
    }
    return static_cast<components_holder<Component>*>(it->second.get())->arr;
}

template <class Component>
inline sparse_array<Component> const &registry::get_components() const {
    auto type_idx = std::type_index(typeid(Component));
    auto it = _components_arrays.find(type_idx);
    if (it == _components_arrays.end()) {
        throw std::runtime_error("Component not registered in registry");
    }
    return static_cast<components_holder<Component> const *>
    (it->second.get())->arr;
}

template <typename Component>
inline typename sparse_array<Component>::reference_type registry::
add_component(entity_t const &to, Component &&c) {
    return get_components<Component>().insert_at(to.getId(),
        std::forward<Component>(c));
}

template <typename Component, typename ... Params>
inline typename sparse_array<Component>::reference_type registry::
emplace_component(entity_t const &to, Params &&... p) {
    return get_components<Component>().emplace_at(to.getId(),
        std::forward<Params>(p)...);
}

template <typename Component>
inline void registry::remove_component(entity_t const &from) {
    get_components<Component>().erase(from.getId());
}

template <typename T>
struct _unwrap_sparse_array { using type = T; };
template <typename U>
struct _unwrap_sparse_array<sparse_array<U>> { using type = U; };

template <class ... Components, typename Function>
inline void registry::add_system(Function &&f) {
    auto wrapper = [func = std::forward<Function>(f)](registry &r) mutable {
        func(r, r.get_components
            <typename _unwrap_sparse_array<Components>::type>()...);
    };
    _systems.emplace_back(std::move(wrapper));
}
}  // namespace Engine
