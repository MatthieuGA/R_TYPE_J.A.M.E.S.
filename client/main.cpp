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

void init_entities(registry &reg) {
    auto entity = reg.spawn_entity();
    reg.emplace_component<Component::Transform>(entity,
        Component::Transform{150.0f, 100.0f, 0, 4.f});
    reg.emplace_component<Component::Drawable>(entity,
        Component::Drawable("ball_enemy.gif", 0, Component::Drawable::CENTER));
    reg.emplace_component<Component::AnimatedSprite>(entity,
        Component::AnimatedSprite{
            17, 17, 12, 0, .1f, 0.f, true
        });
    reg.emplace_component<Component::HitBox>(entity,
        Component::HitBox{17.0f, 17.0f});
    reg.emplace_component<Component::Velocity>(entity,
        Component::Velocity{50.0f, 30.0f, 0.0f, 0.0f});
    reg.emplace_component<Component::PlayerTag>(entity);
    // Create a second entity
    auto entity2 = reg.spawn_entity();
    reg.emplace_component<Component::Transform>(entity2,
        Component::Transform{450.0f, 100.0f, 0, 4.f});
    reg.emplace_component<Component::Drawable>(entity2,
        Component::Drawable("ball_enemy.gif", 0, Component::Drawable::CENTER));
    reg.emplace_component<Component::AnimatedSprite>(entity2,
        Component::AnimatedSprite{
            17, 17, 12, 0, .1f, 0.f, true
        });
    reg.emplace_component<Component::HitBox>(entity2,
        Component::HitBox{17.0f, 17.0f});
    reg.emplace_component<Component::Velocity>(entity2,
        Component::Velocity{-50.0f, 30.0f, 0.0f, 0.0f});
    reg.emplace_component<Component::PlayerTag>(entity2);
}

int main() {
    try {
        registry reg;
        sf::Clock deltaTimeClock;
        sf::RenderWindow window(sf::VideoMode({1280, 920}), "Rtype");

        init_registry(reg, window, deltaTimeClock);
        init_entities(reg);
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
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
