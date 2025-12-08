/**
 * @file ControllablePlayerSystem.cpp
 * @brief System for handling player movement control via input.
 *
 * This system processes player input and applies acceleration to velocity
 * for smooth, responsive movement with gradual speed changes.
 */

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

#include "Engine/initRegistrySystems.hpp"

namespace Rtype::Client {

/**
 * @brief Processes player input and updates velocity with smooth acceleration.
 *
 * @param reg The ECS registry
 * @param inputs Input component array
 * @param controllables Controllable component array
 * @param velocities Velocity component array
 * @param playerTags Player tag component array
 */
void ControllablePlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Inputs> &inputs,
    Eng::sparse_array<Com::Controllable> const &controllables,
    Eng::sparse_array<Com::Velocity> &velocities,
    Eng::sparse_array<Com::PlayerTag> const &playerTags) {
    for (auto &&[i, input, controllable, velocity, playerTag] :
        make_indexed_zipper(inputs, controllables, velocities, playerTags)) {
        if (!controllable.isControllable) continue;

        const float speed = playerTag.speed_max;
        const float time_to_max = 0.15f;  // Time to reach max speed (seconds)
        const float max_accel = speed / time_to_max;

        // Calculate target velocity based on input
        const float target_vx = input.horizontal * speed;
        const float target_vy = input.vertical * speed;

        // Calculate required acceleration to reach target velocity
        const float required_ax = (target_vx - velocity.vx) / time_to_max;
        const float required_ay = (target_vy - velocity.vy) / time_to_max;

        // Clamp acceleration to max_accel
        velocity.accelerationX = std::clamp(required_ax, -max_accel, max_accel);
        velocity.accelerationY = std::clamp(required_ay, -max_accel, max_accel);
    }
}

}  // namespace Rtype::Client
