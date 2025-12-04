#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {
void ControllablePlayerSystem(Eng::registry &reg,
Eng::sparse_array<Com::Inputs> &inputs,
Eng::sparse_array<Com::Controllable> const &controllables,
Eng::sparse_array<Com::Velocity> &velocities,
Eng::sparse_array<Com::PlayerTag> const &playerTags) {
    for (auto &&[i, input, controllable, velocity, playerTag] :
    make_indexed_zipper(inputs, controllables, velocities, playerTags)) {
        if (!controllable.isControllable) continue;

        const float speed = playerTag.speed_max;
        const float time_to_max = 0.15f;
        const float max_accel = speed / time_to_max;

        const float target_vx = input.horizontal * speed;
        const float target_vy = input.vertical * speed;

        const float required_ax = (target_vx - velocity.vx) / time_to_max;
        const float required_ay = (target_vy - velocity.vy) / time_to_max;

        velocity.accelerationX = std::clamp(required_ax, -max_accel, max_accel);
        velocity.accelerationY = std::clamp(required_ay, -max_accel, max_accel);
    }
}
}  // namespace Rtype::Client
