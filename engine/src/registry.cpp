#include "../include/registry.hpp"

#include <iostream>

namespace Engine {
// Entity management
registry::entity_t registry::SpawnEntity() {
    if (!dead_entities_.empty()) {
        std::size_t id = dead_entities_.back();
        dead_entities_.pop_back();
        return entity(id);
    }
    return entity(next_entity_++);
}

registry::entity_t registry::EntityFromIndex(std::size_t idx) {
    return entity(idx);
}

void registry::KillEntity(entity const &e) {
    for (auto &fn : erase_fns_) {
        if (fn)
            fn(*this, e);
    }
    dead_entities_.push_back(e.GetId());
}

void registry::RunSystems() {
    for (size_t i = 0; i < systems_.size(); ++i) {
        auto &s = systems_[i];
        if (s) {
            std::cerr << "S" << i << " " << std::flush;
            s(*this);
        }
    }
    std::cerr << "OK" << std::endl;
}

void registry::ClearAllEntities() {
    // Kill all entities by calling erase functions for all component arrays
    for (std::size_t id = 0; id < next_entity_; ++id) {
        entity e(id);
        for (auto &fn : erase_fns_) {
            if (fn)
                fn(*this, e);
        }
    }
    // Reset entity tracking
    dead_entities_.clear();
    next_entity_ = 0;
}
}  // namespace Engine
