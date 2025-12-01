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
    Engine::sparse_array<Component::Drawable> &drawables,
    Engine::sparse_array<Component::AnimatedSprite> const &animatedSprites);

void movementSystem(Engine::registry &reg, const sf::Clock &deltaTimeClock,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Velocity> &velocities);

void animationSystem(Engine::registry &reg, const sf::Clock &deltaTimeClock,
    Engine::sparse_array<Component::AnimatedSprite> &animatedSprites,
    Engine::sparse_array<Component::Drawable> &drawables);

void playfieldLimitSystem(Engine::registry &reg, const sf::RenderWindow &window,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::PlayerTag> const &playerTags);

void collisionDetectionSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> const &transforms,
    Engine::sparse_array<Component::HitBox> const &hitBoxes);
}  // namespace Rtype::Client
