#pragma once
#include "include/registry.hpp"
#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"

#include "include/indexed_zipper.tpp"

namespace server {
// Frame timing: globals updated each server tick.
extern float g_frame_delta_ms;
extern float g_frame_delta_seconds;

// Minimum delta per frame (enforces maximum 60 FPS).
static constexpr float kMinFrameDeltaSeconds = 1.0f / 60.0f;

/**
 * @brief Update global frame delta from elapsed seconds (clamped to max FPS)
 *
 * @param seconds Elapsed seconds since last frame
 */
void UpdateFrameDeltaFromSeconds(float seconds);

void MovementSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Velocity> &velocities);

void PlayerMovementSystem(Engine::registry &reg,
    Engine::sparse_array<Component::PlayerTag> const &player_tags,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Inputs> const &inputs,
    Engine::sparse_array<Component::Velocity> &velocities);

void PlayerLimitPlayfield(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::PlayerTag> const &player_tags);

void ShootPlayerSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Inputs> &inputs,
    Engine::sparse_array<Component::PlayerTag> &player_tags);

void ProjectileSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Projectile> &projectiles);

void PaternMovementSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Velocity> &velocities,
    Engine::sparse_array<Component::PatternMovement> &patern_movements);

void AnimationSystem(Engine::registry &reg,
    Engine::sparse_array<Component::AnimatedSprite> &anim_sprites);

void TimedEventSystem(Engine::registry &reg,
    Engine::sparse_array<Component::TimedEvents> &timed_events);

void ExplodeOnDeathSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Health> &healths,
    Engine::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Engine::sparse_array<Component::ExplodeOnDeath> &explode_on_deaths,
    Engine::sparse_array<Component::AnimationDeath> &animation_deaths,
    Engine::sparse_array<Component::HitBox> const &hitBoxes,
    Engine::sparse_array<Component::PlayerTag> const &player_tags);

void FrameBaseEventSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Engine::sparse_array<Component::FrameEvents> &frame_events);

void HealthDeductionSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Health> &healths,
    Engine::sparse_array<Component::AnimatedSprite> &animated_sprites,
    Engine::sparse_array<Component::HitBox> const &hitBoxes,
    Engine::sparse_array<Component::Transform> const &transforms,
    Engine::sparse_array<Component::Projectile> const &projectiles);

}  // namespace server
