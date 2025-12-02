#pragma once
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <functional>
#include <utility>
#include <vector>

#include "include/sparse_array.hpp"
#include "include/entity.hpp"

namespace Engine {

struct ComponentsHolderBase {
    virtual ~ComponentsHolderBase() = default;
};

template <typename T>
struct components_holder : ComponentsHolderBase {
    sparse_array<T> arr;
};

class registry {
 public:
    using entity_t = entity;

    template <class Component>
    sparse_array<Component> &RegisterComponent();

    template <class Component>
    sparse_array<Component> &GetComponents();

    template <class Component>
    sparse_array<Component> const &GetComponents() const;

    // Entity management
    entity_t SpawnEntity();
    entity_t EntityFromIndex(std::size_t idx);
    void KillEntity(entity_t const &e);

    // Backwards-compatible entity management
    entity_t spawn_entity() { return SpawnEntity(); }
    entity_t entity_from_index(std::size_t idx) { return EntityFromIndex(idx); }
    void kill_entity(entity_t const &e) { KillEntity(e); }

    template <class ... Components, typename Function>
    void AddSystem(Function &&f);

    void RunSystems();

    // Backwards-compatible run
    void run_systems() { RunSystems(); }

    template <typename Component>
    typename sparse_array<Component>::reference_type
        AddComponent(entity_t const &to, Component &&c);

    template <typename Component>
    typename sparse_array<Component>::reference_type
        add_component(entity_t const &to, Component &&c) {
        return AddComponent<Component>(to, std::forward<Component>(c));
    }

    template <typename Component, typename ... Params>
    typename sparse_array<Component>::reference_type
        EmplaceComponent(entity_t const &to, Params &&... p);

    template <typename Component>
    void RemoveComponent(entity_t const &from);

 private:
    std::unordered_map<std::type_index,
        std::unique_ptr<ComponentsHolderBase>> components_arrays_;
    std::vector<std::function<
        void(registry&, Engine::entity const&)>> erase_fns_;
    std::vector<std::function<void(registry&)>> systems_;
    std::size_t next_entity_ = 0;
    std::vector<std::size_t> dead_entities_;
};

}  // namespace Engine

#include "registry.tpp"

