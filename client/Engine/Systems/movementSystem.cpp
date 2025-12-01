#include "Engine/initRegistrySystems.hpp"

namespace Rtype::Client {
void MovementSystem(Eng::registry &reg, const sf::Clock &delta_time_clock,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Velocity> &velocities) {
    for (auto &&[i, tranform, velocity] :
        make_indexed_zipper(transforms, velocities)) {
        // Update position based on velocity
        tranform.x += velocity.vx * delta_time_clock.getElapsedTime().asSeconds();
        tranform.y += velocity.vy * delta_time_clock.getElapsedTime().asSeconds();
        // Update velocity based on acceleration
        velocity.vx += velocity.accelerationX * delta_time_clock.getElapsedTime().asSeconds();
        velocity.vy += velocity.accelerationY * delta_time_clock.getElapsedTime().asSeconds();
    }
}
}  // namespace Rtype::Client
