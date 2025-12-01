#pragma once
#include <SFML/Graphics.hpp>
#include "include/registry.hpp"
#include "Engine/initRegistryComponent.hpp"
#include "Engine/gameWorld.hpp"


namespace Rtype::Client {
void init_registry_systems(Rtype::Client::GameWorld &gameWorld);
void init_registry_systems_events(Rtype::Client::GameWorld &gameWorld);

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
    Rtype::Client::GameWorld &gameWorld,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::HitBox> const &hitBoxes,
    Engine::sparse_array<Component::Solid> const &solids);
}  // namespace Rtype::Client
