#include "../include/registry.hpp"

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
    for (auto &s : systems_) {
        if (s)
            s(*this);
    }
}
}  // namespace Engine
