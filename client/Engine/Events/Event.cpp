#include "Engine/Events/Event.h"

void EventBus::Unsubscribe(SubId id) {
    for (auto& [type, handlers] : subscribers_) {
        handlers.erase(std::remove_if(handlers.begin(), handlers.end(),
            [id](const Handler& h) { return h.id == id; }),
            handlers.end());
    }
}

void EventBus::Publish(const Event& event) {
    auto it = subscribers_.find(typeid(event));
    if (it != subscribers_.end()) {
        for (auto& handler : it->second)
            handler.fn(event, handler.value);
    }
}

void EventBus::Clear() {
    subscribers_.clear();
    next_id_ = 1;
}