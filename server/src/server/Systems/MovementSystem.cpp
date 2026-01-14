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
        pos.x += vel.vx * g_frame_delta_seconds;
        pos.y += vel.vy * g_frame_delta_seconds;
        vel.vx += vel.accelerationX * g_frame_delta_seconds;
        vel.vy += vel.accelerationY * g_frame_delta_seconds;
    }
}

}  // namespace server
