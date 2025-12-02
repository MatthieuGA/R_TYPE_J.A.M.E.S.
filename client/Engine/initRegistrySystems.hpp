#pragma once
#include <SFML/Graphics.hpp>
#include "include/registry.hpp"
#include "include/indexed_zipper.hpp"
#include "initRegistryComponent.hpp"
#include "gameWorld.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;
void InitRegistrySystems(Rtype::Client::GameWorld &game_world);
void InitRegistrySystemsEvents(Rtype::Client::GameWorld &game_world);

// System function declarations
void DrawableSystem(Eng::registry &reg, sf::RenderWindow &window,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> const &animatedSprites);

void MovementSystem(Eng::registry &reg, const sf::Clock &delta_time_clock,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Velocity> &velocities);

void AnimationSystem(Eng::registry &reg, const sf::Clock &delta_time_clock,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Com::Drawable> &drawables);

void PlayfieldLimitSystem(Eng::registry &reg, const sf::RenderWindow &window,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::PlayerTag> const &player_tags);

void CollisionDetectionSystem(Eng::registry &reg,
    Rtype::Client::GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::HitBox> const &hit_boxes,
    Eng::sparse_array<Com::Solid> const &solids);
}  // namespace Rtype::Client
