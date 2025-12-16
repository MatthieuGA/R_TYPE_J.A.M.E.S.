#include <iostream>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

void PlayerMovementSystem(Engine::registry &reg,
    Engine::sparse_array<Component::PlayerTag> const &player_tags,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::Inputs> const &inputs,
    Engine::sparse_array<Component::Velocity> &velocities) {
    for (auto &&[i, player, velocity, transform, input] :
        make_indexed_zipper(player_tags, velocities, transforms, inputs)) {
        // Get current animation
        const float speed = player.speed_max;
        const float time_to_max = 0.15f;
        const float max_accel = speed / time_to_max;

        const float target_vx = input.horizontal * speed;
        const float target_vy = input.vertical * speed;

        const float required_ax = (target_vx - velocity.vx) / time_to_max;
        const float required_ay = (target_vy - velocity.vy) / time_to_max;

        velocity.accelerationX =
            std::clamp(required_ax, -max_accel, max_accel);
        velocity.accelerationY =
            std::clamp(required_ay, -max_accel, max_accel);

        float rotation = velocity.vx / player.speed_max * 5.f;
        transform.rotationDegrees = rotation;

        for (auto &&[j, child_transform] : make_indexed_zipper(transforms)) {
            if (child_transform.parent_entity.has_value() &&
                child_transform.parent_entity.value() == i) {
                child_transform.rotationDegrees = rotation;
            }
        }
    }
}

}  // namespace server
