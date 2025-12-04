#include <SFML/Graphics.hpp>
#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {
void PlayerSystem(Eng::registry &reg,
Eng::sparse_array<Com::PlayerTag> const &player_tags,
Eng::sparse_array<Com::Velocity> const &velocities,
Eng::sparse_array<Com::AnimatedSprite> &animated_sprites) {
    for (auto &&[i, player_tag, velocity, animated_sprite] :
    make_indexed_zipper(player_tags, velocities, animated_sprites)) {
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
