#include <iostream>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

void MovementSystem(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Velocity> &velocities) {
    for (auto &&[i, pos, vel] : make_indexed_zipper(transforms, velocities)) {
        pos.x += vel.vx * TICK_RATE_MS / 1000.0f;
        pos.y += vel.vy * TICK_RATE_MS / 1000.0f;
        vel.vx += vel.accelerationX * TICK_RATE_MS / 1000.0f;
        vel.vy += vel.accelerationY * TICK_RATE_MS / 1000.0f;
    }
}

}  // namespace server
