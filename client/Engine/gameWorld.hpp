#pragma once
#include <SFML/Graphics.hpp>
#include "include/registry.hpp"
#include "Engine/Events/Event.h"

namespace Rtype::Client {
struct GameWorld {
    Engine::registry registry_;
    sf::RenderWindow window_;
    sf::Clock delta_time_clock_;
    EventBus event_bus_;

    GameWorld()
        : window_(sf::VideoMode({1280, 920}), "Rtype") {
        registry_ = Engine::registry();
        delta_time_clock_ = sf::Clock();
        event_bus_ = EventBus();
    }
};
}  // namespace Rtype::Client
