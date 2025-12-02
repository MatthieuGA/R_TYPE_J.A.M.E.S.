#include <iostream>
#include <cstdio>
#include <vector>
#include <SFML/Graphics.hpp>

#include "include/registry.hpp"
#include "Engine/initRegistryComponent.hpp"
#include "Engine/initRegistrySystems.hpp"
#include "Engine/gameWorld.hpp"

using Engine::registry;
namespace RC = Rtype::Client;
namespace Component = Rtype::Client::Component;

void init_registry(Rtype::Client::GameWorld &game_world) {
    RC::InitRegistryComponents(game_world.registry_);
    RC::InitRegistrySystemsEvents(game_world);
    RC::InitRegistrySystems(game_world);
}

void init_entities(registry &reg) {
    std::vector<registry::entity_t> entities;
    for (int i = 0; i < 3; ++i) {
        auto entity = reg.SpawnEntity();
        reg.EmplaceComponent<Component::Drawable>(entity,
            Component::Drawable("ball_enemy.gif"));
        reg.EmplaceComponent<Component::AnimatedSprite>(entity,
            Component::AnimatedSprite{17, 17, 12, rand() % 12});
        reg.EmplaceComponent<Component::HitBox>(entity,
            Component::HitBox{17.0f, 17.0f});
        entities.push_back(entity);
        reg.EmplaceComponent<Component::PlayerTag>(entity);
    }

    // First entity going down right
    reg.EmplaceComponent<Component::Transform>(entities[0],
        Component::Transform{150.0f, 100.0f, 0, 4.f});
    reg.EmplaceComponent<Component::Solid>(entities[0], Component::Solid{});
    reg.EmplaceComponent<Component::Velocity>(entities[0],
        Component::Velocity{100.0f, 30.0f});

    // Second entity going up left
    reg.EmplaceComponent<Component::Transform>(entities[1],
        Component::Transform{450.0f, 100.0f, 0, 4.f});
    reg.EmplaceComponent<Component::Solid>(entities[1], Component::Solid{});
    reg.EmplaceComponent<Component::Velocity>(entities[1],
        Component::Velocity{-50.0f, 30.0f});

    // Third entity stationary
    reg.EmplaceComponent<Component::Transform>(entities[2],
        Component::Transform{400.f, 250.0f, 0, 4.f});
    reg.EmplaceComponent<Component::Solid>(entities[2],
        Component::Solid{true, true});
}

int main() {
    try {
        Rtype::Client::GameWorld game_world;

        init_registry(game_world);
        init_entities(game_world.registry_);
        while (game_world.window_.isOpen()) {
            sf::Event event;
            while (game_world.window_.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    game_world.window_.close();
            }
            game_world.window_.clear(sf::Color::Black);
            game_world.registry_.RunSystems();
            game_world.window_.display();
        }
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
