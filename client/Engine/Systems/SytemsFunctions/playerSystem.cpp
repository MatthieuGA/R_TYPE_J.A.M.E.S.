/**
 * @file playerSystem.cpp
 * @brief System for managing player animation states based on movement.
 *
 * This system updates the player's animation frame based on their current
 * vertical velocity to provide visual feedback for upward/downward movement.
 */

#include <SFML/Graphics.hpp>

#include "Engine/initRegistrySystems.hpp"

namespace Rtype::Client {

/**
 * @brief Updates player animation frame based on vertical velocity.
 *
 * Animation frames:
 * - Frame 0: Moving very fast downward (vy > 200)
 * - Frame 1: Moving downward (vy >= 75)
 * - Frame 2: Neutral/horizontal movement
 * - Frame 3: Moving upward (vy <= -75)
 * - Frame 4: Moving very fast upward (vy < -200)
 *
 * @param reg The ECS registry
 * @param player_tags Player tag component array
 * @param velocities Velocity component array
 * @param animated_sprites Animated sprite component array
 */
void PlayerSystem(Eng::registry &reg,
    Eng::sparse_array<Com::PlayerTag> const &player_tags,
    Eng::sparse_array<Com::Velocity> const &velocities,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites) {
    for (auto &&[i, player_tag, velocity, animated_sprite] :
        make_indexed_zipper(player_tags, velocities, animated_sprites)) {
        // Update animation frame based on vertical velocity
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
    }
}

}  // namespace Rtype::Client
