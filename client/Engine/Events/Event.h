#pragma once
#include <iostream>
#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>

class EventBus;

struct Event {
    int value;
    virtual ~Event() = default;
};

// EventBus
class EventBus {
public:
    using SubId = std::size_t;

    template<typename T>
    SubId subscribe(std::function<void(const T&, int)> handler) {
        auto& handlers = subscribers[typeid(T)];
        SubId id = nextId++;
        handlers.push_back({
            1,
            id,
            [handler](const Event& e, int value) {
                handler(static_cast<const T&>(e), value);
            }
            });
        return id;
    }

    template<typename T>
    SubId upgradeSubscription(std::function<void(const T&, int)> handler, int valueAdded = 1) {
        auto it = subscribers.find(typeid(T));

        if (it != subscribers.end() && !it->second.empty()) {
            it->second[0].value += valueAdded;
            return it->second[0].id;
        } else {
            return subscribe<T>(handler);
        }
    }

    template<typename T>
    bool isSubscribed() const {
        auto it = subscribers.find(typeid(T));
        return it != subscribers.end() && !it->second.empty();
    }

    void unsubscribe(SubId id);
    void publish(const Event& event);
    void clear();

private:
    struct Handler {
        int value;
        SubId id;
        std::function<void(const Event&, int value)> fn;
    };

    std::unordered_map<std::type_index, std::vector<Handler>> subscribers;
    SubId nextId = 1;
};

