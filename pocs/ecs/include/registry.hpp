#pragma once

#include "entity.hpp"
#include "sparse_array.hpp"
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace ecs {

class registry {
public:
    registry() = default;
    registry(const registry&) = delete;
    registry& operator=(const registry&) = delete;
    registry(registry&&) noexcept = default;
    registry& operator=(registry&&) noexcept = default;
    ~registry() = default;

    // Entity management
    entity spawn_entity() {
        entity e(_next_entity_id++);
        return e;
    }

    void kill_entity(entity e) {
        for (auto& [type, array_ptr] : _components) {
            array_ptr->erase(e.id());
        }
    }

    // Component registration
    template <typename Component>
    sparse_array<Component>& register_component() {
        std::type_index type(typeid(Component));
        
        if (_components.find(type) != _components.end()) {
            return get_components<Component>();
        }

        auto array = std::make_unique<component_array<Component>>();
        auto* array_ptr = array.get();
        _components[type] = std::move(array);
        
        return array_ptr->data;
    }

    // Get component array
    template <typename Component>
    sparse_array<Component>& get_components() {
        std::type_index type(typeid(Component));
        auto it = _components.find(type);
        
        if (it == _components.end()) {
            throw std::runtime_error("Component type not registered");
        }

        return static_cast<component_array<Component>*>(it->second.get())->data;
    }

    template <typename Component>
    const sparse_array<Component>& get_components() const {
        std::type_index type(typeid(Component));
        auto it = _components.find(type);
        
        if (it == _components.end()) {
            throw std::runtime_error("Component type not registered");
        }

        return static_cast<component_array<Component>*>(it->second.get())->data;
    }

    // Add component to entity
    template <typename Component>
    std::optional<Component>& add_component(entity e, const Component& component) {
        return get_components<Component>().insert_at(e.id(), component);
    }

    template <typename Component>
    std::optional<Component>& add_component(entity e, Component&& component) {
        return get_components<Component>().insert_at(e.id(), std::move(component));
    }

    // Emplace component
    template <typename Component, typename... Args>
    std::optional<Component>& emplace_component(entity e, Args&&... args) {
        return get_components<Component>().emplace_at(e.id(), std::forward<Args>(args)...);
    }

    // Remove component
    template <typename Component>
    void remove_component(entity e) {
        get_components<Component>().erase(e.id());
    }

    // Check if entity has component
    template <typename Component>
    bool has_component(entity e) const {
        const auto& components = get_components<Component>();
        return e.id() < components.size() && components[e.id()].has_value();
    }

private:
    struct component_array_base {
        virtual ~component_array_base() = default;
        virtual void erase(std::size_t entity_id) = 0;
    };

    template <typename Component>
    struct component_array : component_array_base {
        sparse_array<Component> data;

        void erase(std::size_t entity_id) override {
            data.erase(entity_id);
        }
    };

    std::size_t _next_entity_id{0};
    std::unordered_map<std::type_index, std::unique_ptr<component_array_base>> _components;
};

} // namespace ecs
