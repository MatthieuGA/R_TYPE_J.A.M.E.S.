#pragma once
#include <SFML/Graphics.hpp>
#include "include/registry.hpp"
#include "Engine/initRegistryComponent.hpp"

namespace Rtype::Client {
void init_registry_systems(Engine::registry &reg, sf::RenderWindow &window);

// System function declarations
void positionSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> const &positions,
    Engine::sparse_array<Component::RigidBody> const &velocities);
void controllableSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Controllable> const &controls);
void drawableSystem(Engine::registry &reg, sf::RenderWindow &window,
    Engine::sparse_array<Component::Transform> const &transforms,
    Engine::sparse_array<Component::Drawable> &drawables);

}  // namespace Rtype::Client
