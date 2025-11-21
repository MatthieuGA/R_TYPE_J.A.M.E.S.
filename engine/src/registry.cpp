#include "../include/registry.hpp"

namespace Engine {
// Entity management
registry::entity_t registry::spawn_entity() {
    if (!_dead_entities.empty()) {
        std::size_t id = _dead_entities.back();
        _dead_entities.pop_back();
        return entity(id);
    }
    return entity(_next_entity++);
}

registry::entity_t registry::entity_from_index(std::size_t idx) {
    return entity(idx);
}

void registry::kill_entity(entity const &e) {
    for (auto &fn : _erase_fns) {
        if (fn) fn(*this, e);
    }
    _dead_entities.push_back(e.getId());
}

void registry::run_systems() {
    for (auto &s : _systems) {
        if (s) s(*this);
    }
}
}  // namespace Engine
