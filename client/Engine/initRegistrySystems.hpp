#pragma once
#include <SFML/Graphics.hpp>
#include "include/registry.hpp"
#include "Engine/initRegistryComponent.hpp"

namespace Rtype::Client {
void init_registry_systems(Engine::registry &reg, sf::RenderWindow &window,
    sf::Clock &deltaTimeClock);

// System function declarations
void drawableSystem(Engine::registry &reg, sf::RenderWindow &window,
    Engine::sparse_array<Component::Transform> const &transforms,
    Engine::sparse_array<Component::Drawable> &drawables);

void movementSystem(Engine::registry &reg, const sf::Clock &deltaTimeClock,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Velocity> &velocities);

}  // namespace Rtype::Client
