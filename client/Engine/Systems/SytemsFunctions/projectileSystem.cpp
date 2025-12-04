#include <SFML/Graphics.hpp>
#include <vector>
#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Update projectiles and remove those outside an extended window.
 *
 * Moves projectiles according to their speed and marks entities for
 * destruction when they leave a region slightly larger than the window.
 *
 * @param reg Engine registry used to kill entities
 * @param game_world Game world providing window size and delta time
 * @param transforms Sparse array of Transform components
 * @param projectiles Sparse array of Projectile components
 */
void ProjectileSystem(Eng::registry &reg, GameWorld &game_world,
Eng::sparse_array<Com::Transform> &transforms,
Eng::sparse_array<Com::Projectile> &projectiles) {
    std::vector<Eng::registry::entity_t> to_kill;
    to_kill.reserve(16);

    for (auto &&[i, transform, projectile] : make_indexed_zipper(transforms, projectiles)) {
        transform.x += projectile.speed * game_world.last_delta_;

        if (transform.x > game_world.window_size_.x + 100.f ||
            transform.x < -100.f ||
            transform.y > game_world.window_size_.y + 100.f ||
            transform.y < -100.f) {
            to_kill.push_back(reg.EntityFromIndex(i));
        }
    }

    for (auto const &e : to_kill) {
        reg.KillEntity(e);
    }
}
}  // namespace Rtype::Client
