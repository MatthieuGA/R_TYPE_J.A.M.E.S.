#pragma once
#include <SFML/Graphics.hpp>
#include "include/registry.hpp"
#include "Engine/Events/Event.h"

namespace Rtype::Client {
struct GameWorld {
    Engine::registry registry;
    sf::RenderWindow window;
    sf::Clock deltaTimeClock;
    EventBus eventBus;

    GameWorld()
        : window(sf::VideoMode({1280, 920}), "Rtype") {
        registry = Engine::registry();
        deltaTimeClock = sf::Clock();
        eventBus = EventBus();
    }
};
}  // namespace Rtype::Client