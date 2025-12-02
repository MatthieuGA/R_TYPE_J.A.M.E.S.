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
    SubId Subscribe(std::function<void(const T&, int)> handler) {
        auto& handlers = subscribers_[typeid(T)];
        SubId id = next_id_++;
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
    SubId UpgradeSubscription(std::function<void(const T&, int)> handler, int valueAdded = 1) {
        auto it = subscribers_.find(typeid(T));

        if (it != subscribers_.end() && !it->second.empty()) {
            it->second[0].value += valueAdded;
            return it->second[0].id;
        } else {
            return subscribe<T>(handler);
        }
    }

    template<typename T>
    bool IsSubscribed() const {
        auto it = subscribers_.find(typeid(T));
        return it != subscribers_.end() && !it->second.empty();
    }

    void Unsubscribe(SubId id);
    void Publish(const Event& event);
    void Clear();

private:
    struct Handler {
        int value;
        SubId id;
        std::function<void(const Event&, int value)> fn;
    };

    std::unordered_map<std::type_index, std::vector<Handler>> subscribers_;
    SubId next_id_ = 1;
};

