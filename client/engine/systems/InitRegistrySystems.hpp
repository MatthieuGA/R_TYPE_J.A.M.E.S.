#pragma once
#include <SFML/Graphics.hpp>

#include "engine/GameWorld.hpp"
#include "engine/InitRegistryComponent.hpp"
#include "engine/audio/AudioManager.hpp"
#include "include/indexed_zipper.hpp"
#include "include/registry.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;
void InitRegistrySystems(
    Rtype::Client::GameWorld &game_world, Audio::AudioManager &audio_manager);
void InitRegistrySystemsEvents(Rtype::Client::GameWorld &game_world);

// System function declarations

// SCENE MANAGEMENT SYSTEMS

void GameStateSystem(Eng::registry &reg, GameWorld &gameWorld,
    Eng::sparse_array<Com::SceneManagement> &sceneManagements);

// RENDER SYSTEMS

void DrawableSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Shader> &shaders,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites,
    Eng::sparse_array<Com::ParticleEmitter> &emitters);

void DrawTextRenderSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Text> &texts);

void AnimationSystem(Eng::registry &reg, const float dt,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Com::Drawable> &drawables);

void LoadAnimationSystem(Eng::registry &reg,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites);

void InitializeShaderSystem(
    Eng::registry &reg, Eng::sparse_array<Com::Shader> &shaders);

void InitializeDrawableAnimatedSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites);

// MOVEMENT SYSTEMS

void InputSystem(Eng::registry &reg, bool has_focus,
    Eng::sparse_array<Com::Inputs> &inputs);

void MovementSystem(Eng::registry &reg, const float dt,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Velocity> &velocities);

void ParallaxSystem(Eng::registry &reg, const GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::ParrallaxLayer> const &parallax_layers,
    Eng::sparse_array<Com::Drawable> const &drawables);

void PlayfieldLimitSystem(Eng::registry &reg, const GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::PlayerTag> const &player_tags);

void CollisionDetectionSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::HitBox> const &hit_boxes,
    Eng::sparse_array<Com::Solid> const &solids);

void ControllablePlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Inputs> &inputs,
    Eng::sparse_array<Com::Controllable> const &controllables,
    Eng::sparse_array<Com::Velocity> &velocities,
    Eng::sparse_array<Com::PlayerTag> const &playerTags);

void AnimationEnterPlayerSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Velocity> &velocities,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::PlayerTag> &player_tags,
    Eng::sparse_array<Com::AnimationEnterPlayer> &animation_enter_players);

// GAMEPLAY SYSTEMS

void ChargingShowAssetPlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::PlayerTag> &player_tags,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Com::Transform> &transforms);

void PlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::PlayerTag> const &player_tags,
    Eng::sparse_array<Com::Velocity> const &velocities,
    Eng::sparse_array<Com::Inputs> const &inputs,
    Eng::sparse_array<Com::ParticleEmitter> &particle_emitters,
    Eng::sparse_array<Com::Transform> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites);

void ShootPlayerSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Inputs> const &inputs,
    Eng::sparse_array<Com::PlayerTag> &player_tags);

void ProjectileSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Projectile> &projectiles);

void ButtonClickSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::HitBox> &hit_boxes,
    Eng::sparse_array<Com::Clickable> &clickables,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Transform> &transforms);

void HealthDeductionSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Health> &healths,
    Eng::sparse_array<Com::HealthBar> &health_bars,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Com::HitBox> const &hitBoxes,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Projectile> const &projectiles);

void DeathAnimationSystem(Eng::registry &reg,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Com::AnimationDeath> &animation_deaths);

void FrameBaseEventSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Com::FrameEvents> &frame_events);

void PlayerHitSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Eng::sparse_array<Com::PlayerTag> &player_hits);

void TimedEventSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::TimedEvents> &timed_events);

void PaternMovementSystem(Eng::registry &reg, const float dt,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Velocity> &velocities,
    Eng::sparse_array<Com::PatternMovement> &patern_movements);

void HealthBarSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::HealthBar> &health_bars,
    Eng::sparse_array<Com::Health> const &healths);
// AUDIO SYSTEM

void AudioSystem(Eng::registry &reg, Audio::AudioManager &audio_manager,
    Eng::sparse_array<Com::SoundRequest> &sound_requests);

}  // namespace Rtype::Client
