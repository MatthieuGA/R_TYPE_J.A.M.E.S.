#include <iostream>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/Server.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

void Server::RegisterSystems() {
    registry_.AddSystem<Engine::sparse_array<Component::AnimatedSprite>>(
        AnimationSystem);

    registry_.AddSystem<Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::AnimatedSprite>,
        Engine::sparse_array<Component::FrameEvents>>(FrameBaseEventSystem);

    registry_.AddSystem<Engine::sparse_array<Component::TimedEvents>>(
        TimedEventSystem);

    registry_.AddSystem<Engine::sparse_array<Component::PlayerTag>,
        Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::Inputs>,
        Engine::sparse_array<Component::Velocity>>(PlayerMovementSystem);

    registry_.AddSystem<Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::Velocity>,
        Engine::sparse_array<Component::PatternMovement>>(
        PaternMovementSystem);

    registry_.AddSystem<Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::Velocity>>(MovementSystem);

    registry_.AddSystem<Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::PlayerTag>>(PlayerLimitPlayfield);

    registry_.AddSystem<Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::Projectile>>(ProjectileSystem);

    registry_.AddSystem<Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::Inputs>,
        Engine::sparse_array<Component::PlayerTag>>(ShootPlayerSystem);

    registry_.AddSystem<Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::PlayerTag>>(PlayerGatlingSystem);

    registry_.AddSystem<Engine::sparse_array<Component::Health>,
        Engine::sparse_array<Component::AnimatedSprite>,
        Engine::sparse_array<Component::HitBox>,
        Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::Projectile>,
        Engine::sparse_array<Component::DeflectedProjectiles>>(
        HealthDeductionSystem);

    // Obstacle collision system - handles player vs obstacle collisions
    // Must run after movement systems to ensure positions are up to date
    registry_.AddSystem<Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::HitBox>,
        Engine::sparse_array<Component::PlayerTag>,
        Engine::sparse_array<Component::ObstacleTag>,
        Engine::sparse_array<Component::AnimatedSprite>>(
        ObstacleCollisionSystem);

    // Explode-on-death system must run after health deduction to catch deaths
    registry_.AddSystem<Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::Health>,
        Engine::sparse_array<Component::AnimatedSprite>,
        Engine::sparse_array<Component::ExplodeOnDeath>,
        Engine::sparse_array<Component::AnimationDeath>,
        Engine::sparse_array<Component::HitBox>,
        Engine::sparse_array<Component::PlayerTag>>(ExplodeOnDeathSystem);

    // Despawn offscreen system - removes entities that moved past the left
    // edge Must run after movement systems to ensure positions are up to date
    registry_.AddSystem<Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::PlayerTag>>(DespawnOffscreenSystem);
}

}  // namespace server
