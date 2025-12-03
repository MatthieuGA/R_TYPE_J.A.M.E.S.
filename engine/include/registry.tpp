#pragma once
#include "registry.hpp"
#include <stdexcept>

namespace Engine {

// Template definitions (must be in header for linkage)
template <class Component>
inline sparse_array<Component> &registry::RegisterComponent() {
    auto type_idx = std::type_index(typeid(Component));
    auto it = components_arrays_.find(type_idx);
    if (it == components_arrays_.end()) {
        auto holder = std::make_unique<components_holder<Component>>();
        components_arrays_[type_idx] = std::move(holder);
        std::function<void(registry&, entity const&)> fn =
            [](registry &r, entity const &e) {
        try {
            r.GetComponents<Component>().erase(e.GetId());
        } catch (...) {}
        };
        erase_fns_.push_back(std::move(fn));
    } else {
        throw std::runtime_error("Component already registered in registry");
    }
    return static_cast<components_holder<Component>*>
        (components_arrays_[type_idx].get())->arr;
}

template <class Component>
inline sparse_array<Component> &registry::GetComponents() {
    auto type_idx = std::type_index(typeid(Component));
    auto it = components_arrays_.find(type_idx);
    if (it == components_arrays_.end()) {
        throw std::runtime_error("Component not registered in registry");
    }
    return static_cast<components_holder<Component>*>(it->second.get())->arr;
}

template <class Component>
inline sparse_array<Component> const &registry::GetComponents() const {
    auto type_idx = std::type_index(typeid(Component));
    auto it = components_arrays_.find(type_idx);
    if (it == components_arrays_.end()) {
        throw std::runtime_error("Component not registered in registry");
    }
    return static_cast<components_holder<Component> const *>
    (it->second.get())->arr;
}

template <typename Component>
inline typename sparse_array<Component>::reference_type registry::
AddComponent(entity_t const &to, Component &&c) {
    return GetComponents<Component>().insert_at(to.GetId(),
        std::forward<Component>(c));
}

template <typename Component, typename ... Params>
inline typename sparse_array<Component>::reference_type registry::
EmplaceComponent(entity_t const &to, Params &&... p) {
    return GetComponents<Component>().emplace_at(to.GetId(),
        std::forward<Params>(p)...);
}

template <typename Component>
inline void registry::RemoveComponent(entity_t const &from) {
    GetComponents<Component>().erase(from.GetId());
}

template <typename T>
struct _unwrap_sparse_array { using type = T; };
template <typename U>
struct _unwrap_sparse_array<sparse_array<U>> { using type = U; };

template <class ... Components, typename Function>
inline void registry::AddSystem(Function &&f) {
    auto wrapper = [func = std::forward<Function>(f)](registry &r) mutable {
        func(r, r.GetComponents
            <typename _unwrap_sparse_array<Components>::type>()...);
    };
    systems_.emplace_back(std::move(wrapper));
}
}  // namespace Engine
