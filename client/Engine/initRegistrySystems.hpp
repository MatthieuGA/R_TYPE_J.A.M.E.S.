#pragma once
#include <SFML/Graphics.hpp>
#include "include/registry.hpp"
#include "Engine/initRegistryComponent.hpp"
#include "Engine/gameWorld.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;
void init_registry_systems(Rtype::Client::GameWorld &gameWorld);
void init_registry_systems_events(Rtype::Client::GameWorld &gameWorld);

// System function declarations
void drawableSystem(Eng::registry &reg, sf::RenderWindow &window,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> const &animatedSprites);

void movementSystem(Eng::registry &reg, const sf::Clock &deltaTimeClock,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Velocity> &velocities);

void animationSystem(Eng::registry &reg, const sf::Clock &deltaTimeClock,
    Eng::sparse_array<Com::AnimatedSprite> &animatedSprites,
    Eng::sparse_array<Com::Drawable> &drawables);

void playfieldLimitSystem(Eng::registry &reg, const sf::RenderWindow &window,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::PlayerTag> const &playerTags);

void collisionDetectionSystem(Eng::registry &reg,
    Rtype::Client::GameWorld &gameWorld,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::HitBox> const &hitBoxes,
    Eng::sparse_array<Com::Solid> const &solids);
}  // namespace Rtype::Client
