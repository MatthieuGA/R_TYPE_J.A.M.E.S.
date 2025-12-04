#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Apply velocities and accelerations to transforms.
 *
 * Updates entity positions using current velocities and updates velocities
 * using the stored acceleration values.
 *
 * @param reg Engine registry (unused)
 * @param dt Delta time (seconds)
 * @param transforms Sparse array of Transform components to update
 * @param velocities Sparse array of Velocity components
 */
void MovementSystem(Eng::registry &reg, const float dt,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Velocity> &velocities) {
    for (auto &&[i, tranform, velocity] :
        make_indexed_zipper(transforms, velocities)) {
        // Update position based on velocity
        tranform.x += velocity.vx * dt;
        tranform.y += velocity.vy * dt;
        // Update velocity based on acceleration
        velocity.vx += velocity.accelerationX * dt;
        velocity.vy += velocity.accelerationY * dt;
    }
}
}  // namespace Rtype::Client
