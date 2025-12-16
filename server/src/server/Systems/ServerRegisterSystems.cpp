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
}

}  // namespace server
