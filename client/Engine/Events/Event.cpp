#include "Engine/Events/Event.h"

void EventBus::unsubscribe(SubId id) {
    for (auto& [type, handlers] : subscribers) {
        handlers.erase(std::remove_if(handlers.begin(), handlers.end(),
            [id](const Handler& h) { return h.id == id; }),
            handlers.end());
    }
}

void EventBus::publish(const Event& event) {
    auto it = subscribers.find(typeid(event));
    if (it != subscribers.end()) {
        for (auto& handler : it->second)
            handler.fn(event, handler.value);
    }
}

void EventBus::clear() {
    subscribers.clear();
    nextId = 1;
}