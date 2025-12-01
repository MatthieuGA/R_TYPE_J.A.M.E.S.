#include <iostream>
#include <cstdio>
#include <SFML/Graphics.hpp>

#include "include/registry.hpp"
#include "Engine/initRegistryComponent.hpp"
#include "Engine/initRegistrySystems.hpp"
#include "Engine/gameWorld.hpp"

using Engine::registry;
namespace RC = Rtype::Client;
namespace Component = Rtype::Client::Component;

void init_registry(Rtype::Client::GameWorld &gameWorld) {
    RC::init_registry_components(gameWorld.registry);
    RC::init_registry_systems_events(gameWorld);
    RC::init_registry_systems(gameWorld);
}

void init_entities(registry &reg) {
    std::vector<registry::entity_t> entities;
    for (int i = 0; i < 3; ++i) {
        auto entity = reg.spawn_entity();
        reg.emplace_component<Component::Drawable>(entity,
            Component::Drawable("ball_enemy.gif"));
        reg.emplace_component<Component::AnimatedSprite>(entity,
            Component::AnimatedSprite{17, 17, 12, rand() % 12});
        reg.emplace_component<Component::HitBox>(entity,
            Component::HitBox{17.0f, 17.0f});
        entities.push_back(entity);
        reg.emplace_component<Component::PlayerTag>(entity);
    }

    reg.emplace_component<Component::Transform>(entities[0],
        Component::Transform{150.0f, 10.0f, 0, 4.f});
    reg.emplace_component<Component::Solid>(entities[0], Component::Solid{});
    reg.emplace_component<Component::Velocity>(entities[0],
        Component::Velocity{300.0f, 30.0f});

    // Create a second entity
    reg.emplace_component<Component::Transform>(entities[1],
        Component::Transform{450.0f, 100.0f, 0, 4.f});
    reg.emplace_component<Component::Solid>(entities[1], Component::Solid{});
    reg.emplace_component<Component::Velocity>(entities[1],
        Component::Velocity{-150.0f, 30.0f});

    reg.emplace_component<Component::Transform>(entities[2],
        Component::Transform{375.0f, 300.0f, 0, 4.f});
    reg.emplace_component<Component::Solid>(entities[2],
        Component::Solid{true, true});
}

int main() {
    try {
        Rtype::Client::GameWorld gameWorld;

        init_registry(gameWorld);
        init_entities(gameWorld.registry);
        while (gameWorld.window.isOpen()) {
            sf::Event event;
            while (gameWorld.window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    gameWorld.window.close();
            }
            gameWorld.window.clear(sf::Color::Black);
            gameWorld.registry.run_systems();
            gameWorld.window.display();
        }
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
