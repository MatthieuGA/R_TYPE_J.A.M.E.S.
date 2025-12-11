#include <SFML/Graphics.hpp>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Update player animated sprite frame selection based on vertical
 * velocity.
 *
 * Chooses a frame index representing up/down/neutral movement states.
 *
 * @param reg Engine registry (unused)
 * @param player_tags Sparse array of PlayerTag components
 * @param velocities Sparse array of Velocity components
 * @param animated_sprites Sparse array of AnimatedSprite components to update
 */
void PlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::PlayerTag> const &player_tags,
    Eng::sparse_array<Com::Velocity> const &velocities,
    Eng::sparse_array<Com::Inputs> const &inputs,
    Eng::sparse_array<Com::ParticleEmitter> &particle_emitters,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites) {
    for (auto &&[i, player_tag, velocity, animated_sprite, transform] :
        make_indexed_zipper(
            player_tags, velocities, animated_sprites, transforms)) {
        // Get current animation
        auto it =
            animated_sprite.animations.find(animated_sprite.currentAnimation);
        if (it == animated_sprite.animations.end()) {
            it = animated_sprite.animations.find("Default");
            if (it == animated_sprite.animations.end())
                continue;
        }
        auto &animation = it->second;

        if (velocity.vy > 200.f)
            animation.current_frame = 0;
        else if (velocity.vy >= 75)
            animation.current_frame = 1;
        else if (velocity.vy < -200.f)
            animation.current_frame = 4;
        else if (velocity.vy <= -75)
            animation.current_frame = 3;
        else
            animation.current_frame = 2;

        float rotation = velocity.vx / player_tag.speed_max * 5.f;
        transform.rotationDegrees = rotation;

        if (inputs.has(i) && particle_emitters.has(i)) {
            auto const &input = inputs[i];
            auto &emitter = particle_emitters[i];

            emitter->emitting = input->horizontal > 0.0f;
        }

        for (auto &&[j, child_transform] : make_indexed_zipper(transforms)) {
            if (child_transform.parent_entity.has_value() &&
                child_transform.parent_entity.value() == i) {
                child_transform.rotationDegrees = rotation;
            }
        }
    }
}
}  // namespace Rtype::Client
