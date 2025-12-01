#include <iostream>
#include "Engine/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;
void movementSystem(Eng::registry &reg, const sf::Clock &deltaTimeClock,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::Velocity> &velocities) {
    for (auto &&[i, tranform, velocity] :
        make_indexed_zipper(transforms, velocities)) {
        // Update position based on velocity
        tranform.x += velocity.vx * deltaTimeClock.getElapsedTime().asSeconds();
        tranform.y += velocity.vy * deltaTimeClock.getElapsedTime().asSeconds();
        // Update velocity based on acceleration
        velocity.vx += velocity.accelerationX * deltaTimeClock.getElapsedTime().asSeconds();
        velocity.vy += velocity.accelerationY * deltaTimeClock.getElapsedTime().asSeconds();
    }
}
}  // namespace Rtype::Client
