#include <iostream>
#include <cstdio>
#include <SFML/Graphics.hpp>

#include "include/registry.hpp"
#include "Engine/initRegistryComponent.hpp"
#include "Engine/initRegistrySystems.hpp"

using Engine::registry;
namespace RC = Rtype::Client;
namespace Component = Rtype::Client::Component;

void init_registry(registry &reg, sf::RenderWindow &window, sf::Clock &deltaTimeClock) {
    RC::init_registry_components(reg);
    RC::init_registry_systems(reg, window, deltaTimeClock);
}

int main() {
    registry reg;
    sf::Clock deltaTimeClock;
    sf::RenderWindow window(sf::VideoMode({1280, 920}), "SFML");
    init_registry(reg, window, deltaTimeClock);

    auto entity = reg.spawn_entity();
    reg.emplace_component<Component::Transform>(entity,
        Component::Transform{150.0f, 100.0f, 0, 1.f});
    reg.emplace_component<Component::Drawable>(entity,
        Component::Drawable("ball_enemy.gif", 0, Component::Drawable::CENTER));
    reg.emplace_component<Component::Velocity>(entity,
        Component::Velocity{10.0f, 1.5f, 20.0f, 0.0f});

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::Black);
        reg.run_systems();
        window.display();
    }

    return 0;
}
