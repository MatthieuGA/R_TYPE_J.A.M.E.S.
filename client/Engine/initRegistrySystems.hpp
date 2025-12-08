#pragma once
#include <memory>

#include <SFML/Graphics.hpp>

#include "Engine/Audio/AudioManager.hpp"
#include "Engine/gameWorld.hpp"
#include "Engine/initRegistryComponent.hpp"
#include "include/indexed_zipper.hpp"
#include "include/registry.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;
void InitRegistrySystems(Rtype::Client::GameWorld &game_world,
    Audio::AudioManager &audio_manager);
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

void AudioSystem(Eng::registry &reg, Audio::AudioManager &audio_manager,
    Eng::sparse_array<Com::SoundRequest> &sound_requests);

void ControllablePlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Inputs> &inputs,
    Eng::sparse_array<Com::Controllable> const &controllables,
    Eng::sparse_array<Com::Velocity> &velocities,
    Eng::sparse_array<Com::PlayerTag> const &playerTags);

void PlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::PlayerTag> const &player_tags,
    Eng::sparse_array<Com::Velocity> const &velocities,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites);
}  // namespace Rtype::Client
