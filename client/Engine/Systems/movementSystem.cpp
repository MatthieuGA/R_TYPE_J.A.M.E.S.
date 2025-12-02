#include "Engine/initRegistrySystems.hpp"

namespace Rtype::Client {
void MovementSystem(Eng::registry &reg, const sf::Clock &delta_time_clock,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Velocity> &velocities) {
    for (auto &&[i, tranform, velocity] :
        make_indexed_zipper(transforms, velocities)) {
        float dt = delta_time_clock.getElapsedTime().asSeconds();
        // Update position based on velocity
        tranform.x += velocity.vx * dt;
        tranform.y += velocity.vy * dt;
        // Update velocity based on acceleration
        velocity.vx += velocity.accelerationX * dt;
        velocity.vy += velocity.accelerationY * dt;
    }
}
}  // namespace Rtype::Client
