#include <SFML/Graphics.hpp>

#include "engine/systems/initRegistrySystems.hpp"

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
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites) {
    for (auto &&[i, player_tag, velocity, animated_sprite, transform] :
        make_indexed_zipper(
            player_tags, velocities, animated_sprites, transforms)) {
        if (velocity.vy > 200.f)
            animated_sprite.currentFrame = 0;
        else if (velocity.vy >= 75)
            animated_sprite.currentFrame = 1;
        else if (velocity.vy < -200.f)
            animated_sprite.currentFrame = 4;
        else if (velocity.vy <= -75)
            animated_sprite.currentFrame = 3;
        else
            animated_sprite.currentFrame = 2;

        float rotation = velocity.vx / player_tag.speed_max * 5.f;
        transform.rotationDegrees = rotation;

        for (auto &&[j, child_transform] : make_indexed_zipper(transforms)) {
            if (child_transform.parent_entity.has_value() &&
                child_transform.parent_entity.value() == i) {
                child_transform.rotationDegrees = rotation;
            }
        }
    }
}
}  // namespace Rtype::Client
