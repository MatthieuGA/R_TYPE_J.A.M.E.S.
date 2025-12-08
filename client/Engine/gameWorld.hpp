#pragma once
#include <SFML/Graphics.hpp>

#include "include/WindowConst.hpp"
#include "include/registry.hpp"

#include "Engine/Events/Event.h"

namespace Rtype::Client {
struct GameWorld {
    Engine::registry registry_;
    sf::RenderWindow window_;
    sf::Vector2f window_size_;
    sf::Clock delta_time_clock_;
    sf::Clock total_time_clock_;
    float last_delta_ = 0.0f;
    EventBus event_bus_;

    GameWorld()
        : window_(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), WINDOW_TITLE) {
        registry_ = Engine::registry();
        delta_time_clock_ = sf::Clock();
        event_bus_ = EventBus();
        window_size_ = sf::Vector2f({WINDOW_WIDTH, WINDOW_HEIGHT});
    }
};
}  // namespace Rtype::Client
