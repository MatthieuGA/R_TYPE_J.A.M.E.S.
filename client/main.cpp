#include <iostream>
#include <cstdio>
#include <SFML/Graphics.hpp>

#include "include/registry.hpp"
#include "Engine/initRegisteryComponent.hpp"
#include "Engine/initRegisterySystems.hpp"

using Engine::registry;
namespace RC = Rtype::Client;
namespace Component = Rtype::Client::Component;

void init_registry(registry &reg, sf::RenderWindow &window) {
    RC::init_registry_components(reg);
    RC::init_registry_systems(reg, window);
}

int main() {
    registry reg;
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML");
    init_registry(reg, window);

    for (int i = 0; i < 4; ++i) {
        auto entity = reg.spawn_entity();
        reg.emplace_component<Component::Transform>(entity,
            Component::Transform{(i+1) * 150.0f, 100.0f, i * 10.f, 0.2f});
        reg.emplace_component<Component::Drawable>(entity,
            Component::Drawable("Logo.png", 0, Component::Drawable::CENTER));
    }

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color::Black);
        reg.run_systems();
        window.display();
    }

    return 0;
}
