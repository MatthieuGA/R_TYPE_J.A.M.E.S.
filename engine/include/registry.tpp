#pragma once
#include "registry.hpp"
#include <stdexcept>

namespace Engine {

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
            r.get_components<Component>().erase(e.get_id());
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
    return get_components<Component>().insert_at(to.get_id(),
        std::forward<Component>(c));
}

template <typename Component, typename ... Params>
inline typename sparse_array<Component>::reference_type registry::
emplace_component(entity_t const &to, Params &&... p) {
    return get_components<Component>().emplace_at(to.get_id(),
        std::forward<Params>(p)...);
}

template <typename Component>
inline void registry::remove_component(entity_t const &from) {
    get_components<Component>().erase(from.get_id());
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
