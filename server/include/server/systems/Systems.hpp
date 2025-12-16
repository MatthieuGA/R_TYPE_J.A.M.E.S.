#pragma once
#include "include/registry.hpp"
#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"

#include "include/indexed_zipper.tpp"

namespace server {
const constexpr float TICK_RATE_MS = 16.0f;
const constexpr float TICK_RATE_SECONDS = TICK_RATE_MS / 1000.0f;

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

}  // namespace server
