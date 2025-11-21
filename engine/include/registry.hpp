#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <functional>
#include <utility>
#include <vector>

#include "sparse_array.hpp"
#include "entity.hpp"

namespace Engine {

struct components_holder_base {
    virtual ~components_holder_base() = default;
};

template <typename T>
struct components_holder : components_holder_base {
    sparse_array<T> arr;
};

class registry {
 public:
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
    std::unordered_map<std::type_index,
        std::unique_ptr<components_holder_base>> _components_arrays;
    std::vector<std::function<
        void(registry&, Engine::entity const&)>> _erase_fns;
    std::vector<std::function<void(registry&)>> _systems;
    std::size_t _next_entity = 0;
    std::vector<std::size_t> _dead_entities;
};

}  // namespace Engine

#include "registry.tpp"
